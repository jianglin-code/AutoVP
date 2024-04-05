package com.android.server;

import android.os.ICellsService;
import android.content.Context;
import android.os.SystemProperties;
import android.os.Looper;
import android.os.Bundle;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.IBinder;
import android.os.INetworkManagementService;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.os.Message;
import android.os.SystemClock;
import android.os.Process;
import android.util.Log;

import android.net.ConnectivityManager;
//import android.net.DhcpResults;
import android.net.Network;
//import android.net.dhcp.DhcpClient;
import android.net.InterfaceConfiguration;
import android.net.LinkAddress;
import android.net.LinkProperties;
import android.net.NetworkAgent;
//import android.net.IConnectivityManager;
import android.net.NetworkCapabilities;
import android.net.NetworkFactory;
import android.net.NetworkInfo;
import android.net.NetworkInfo.DetailedState;
import android.net.NetworkRequest;
import android.net.NetworkProvider;
import android.net.RouteInfo;
import android.net.StaticIpConfiguration;
import android.net.NetworkAgentConfig;
import android.net.TrafficStats;
import android.net.InetAddresses;
import android.net.IpPrefix;
import android.app.CellsPrivateServiceMgr;
import android.provider.Settings;

import com.android.server.net.NetlinkTracker;
import com.android.internal.util.StateMachine;
import com.android.internal.util.State;

import java.util.regex.Matcher;
import java.util.regex.Pattern;
import java.util.List;
import java.net.Inet4Address;
import java.net.Inet6Address;
import java.net.InetAddress;

public class CellsService extends ICellsService.Stub {

    private final Context mContext;
    private boolean mSystemReady = false;

    private CellsNetworkAgent mCellsNetworkAgent;

    public CellsService(Context context) {
        mContext = context;
    }

    @Override
    public boolean isSystemReady(){
        return mSystemReady;
    }

    @Override
    public LinkProperties getActiveLinkProperties(){
        ConnectivityManager mConnmanager = mContext.getSystemService(ConnectivityManager.class);
        if(mConnmanager != null && mConnmanager.getActiveNetwork() != null){
            LinkProperties linkProperties = mConnmanager.getLinkProperties(mConnmanager.getActiveNetwork());
            if(linkProperties != null){
                return linkProperties;
            }
        }

        return null;
    }

    @Override
    public NetworkInfo getActiveNetworkInfo(){
        ConnectivityManager mConnmanager = mContext.getSystemService(ConnectivityManager.class);
        if(mConnmanager != null){
            NetworkInfo networkInfo = mConnmanager.getActiveNetworkInfo();
            if(networkInfo != null){
                return networkInfo;
            }
        }

        return null;
    }

    public  void systemReady(){
        if(SystemProperties.get("ro.boot.vm","0").equals("1")){
            if(SystemProperties.get("persist.sys.cells.netagent","").equals("")){
                mCellsNetworkAgent = new CellsNetworkAgent(mContext,this);
            }

            Thread vmready = new Thread(new Runnable(){
                @Override
                public void run() {
                    CellsPrivateServiceMgr mCellsService = new CellsPrivateServiceMgr(mContext,
                                ServiceManager.getInitService("CellsPrivateService"));
                    
                    do
                    {
                        SystemClock.sleep(3000);

                        if("1".equals(SystemProperties.get("sys.boot_completed"))){
                            SystemClock.sleep(500);
                            mCellsService.vmSystemReady();
                            return ;
                        }
                    }while(true);
                }
            });
            vmready.setDefaultUncaughtExceptionHandler(null);
            vmready.start();
        }

        mSystemReady = true;
    }

    private class CellsNetworkAgent {
        private static final String LOG_TAG = "CellsNetworkAgent";
        private final Context mContext;
        private final CellsService mCells;

        private NetworkAgent mSystemAgent = null;
        private Handler mSystemAgentHandler;

        private static final String VM_INTERFACENAME = "wlan0";
        private static final String NETWORKTYPE = "WIFI";
        private static final String TCPBUFFERSIZES = "524288,2097152,4194304,262144,524288,1048576";

        private static final int VM_BASE_ADDR                  = 16 ;
        private static final String VM_NEAR_IFACE_ADDR         = "172.%d.3.3";
        private static final String VM_GATEWAY_ADDR            = "172.%d.3.2";
        private static final String VM_DEST_ADDR               = "172.%d.0.0";
        private static final int VM_PREFIX_LENGTH              = 16;

        private NetworkInfo mNetworkInfo;
        private NetworkCapabilities mNetworkCapabilities;
        private LinkProperties mLinkProperties;
        private int mScore;
        private NetworkAgentConfig mNetworkAgentConfig;
        private NetworkProvider mNetworkProvider;
        private ConnectivityManager mConnectivityManager;

        private int mIndex = 0;

        private LinkProperties reBuildVMLinkProperties(int index){
            mLinkProperties = new LinkProperties();

            mLinkProperties.setInterfaceName(VM_INTERFACENAME);
            mLinkProperties.setTcpBufferSizes(TCPBUFFERSIZES);

            {
                InetAddress gateway = InetAddresses.parseNumericAddress(String.format(VM_GATEWAY_ADDR,VM_BASE_ADDR + index));
                InetAddress dest    = InetAddresses.parseNumericAddress("0.0.0.0");
                RouteInfo route = new RouteInfo(new IpPrefix(dest, 0),gateway,mLinkProperties.getInterfaceName(),RouteInfo.RTN_UNICAST);
                mLinkProperties.addRoute(route);
            }

            {
                InetAddress gateway = InetAddresses.parseNumericAddress("0.0.0.0");
                InetAddress dest    = InetAddresses.parseNumericAddress(String.format(VM_DEST_ADDR,VM_BASE_ADDR + index));
                RouteInfo route = new RouteInfo(new IpPrefix(dest,VM_PREFIX_LENGTH),gateway,mLinkProperties.getInterfaceName(),RouteInfo.RTN_UNICAST);
                mLinkProperties.addRoute(route);
            }

            {
                IBinder b = ServiceManager.getInitService(Context.CELLS_SERVICE);
                if(b != null){
                    ICellsService service = ICellsService.Stub.asInterface(b);

                    try {
                        LinkProperties linkProperties = service.getActiveLinkProperties();
                        if(linkProperties != null){
                            for (InetAddress dns : linkProperties.getDnsServers()) {
                                //Log.d(LOG_TAG, "getDnsServers: " + dns.toString());
                                //if (dns instanceof Inet4Address) 
                                {
                                    Log.d(LOG_TAG, "DNS Inet4Address: " + dns.toString());
                                    mLinkProperties.addDnsServer(dns);
                                }
                            }
                        }   
                    } catch (RemoteException e) {
                        Log.e(LOG_TAG, "Couldn't getActiveLinkProperties: " + e.toString());
                    }
                }
            }

            InetAddress addr = InetAddresses.parseNumericAddress(String.format(VM_NEAR_IFACE_ADDR,VM_BASE_ADDR + index));
            mLinkProperties.addLinkAddress(new LinkAddress(addr, VM_PREFIX_LENGTH));

            return mLinkProperties;
        }

        private NetworkCapabilities reBuildVMNetworkCapabilities(int index){
            mNetworkCapabilities = new NetworkCapabilities.Builder()
                .addTransportType(NetworkCapabilities.TRANSPORT_WIFI)
                .addCapability(NetworkCapabilities.NET_CAPABILITY_INTERNET)
                .addCapability(NetworkCapabilities.NET_CAPABILITY_NOT_RESTRICTED)
                .addCapability(NetworkCapabilities.NET_CAPABILITY_TRUSTED)
                .addCapability(NetworkCapabilities.NET_CAPABILITY_NOT_VPN)
                .addCapability(NetworkCapabilities.NET_CAPABILITY_NOT_VCN_MANAGED)
                .setLinkUpstreamBandwidthKbps(1024 * 1024)
                .setLinkDownstreamBandwidthKbps(1024 * 1024)
                .setSignalStrength(-60)
                .build();
            return mNetworkCapabilities;
        }

        private NetworkInfo reBuildVMNetworkInfo(int index){
            mNetworkInfo = new NetworkInfo(ConnectivityManager.TYPE_WIFI, 0, NETWORKTYPE, "");
            mNetworkInfo.setDetailedState(DetailedState.CONNECTED, null, "cells-ap");
            return mNetworkInfo;
        }

        private int reBuildVMScore(int index){
            mScore = 60;
            return mScore;
        }

        private NetworkAgentConfig reBuildVMNetworkAgentConfig(int index){
            mNetworkAgentConfig = new NetworkAgentConfig.Builder()
                .setLegacyType(ConnectivityManager.TYPE_WIFI)
                .setLegacyTypeName(NETWORKTYPE)
                .setExplicitlySelected(false)
                .setUnvalidatedConnectivityAcceptable(false)
                .setPartialConnectivityAcceptable(false)
                .build();
            return mNetworkAgentConfig;
        }

        private NetworkProvider reBuildVMNetworkProvider(int index, Handler handler){
            mNetworkProvider = new NetworkProvider(mContext, handler.getLooper(), LOG_TAG);
            mConnectivityManager.registerNetworkProvider(mNetworkProvider);
            return mNetworkProvider;
        }

        private void createVMNetworkAgent(final int index){
            if(mSystemAgentHandler == null)
                return;

            mSystemAgent = new NetworkAgent(mContext, mSystemAgentHandler.getLooper(), "CellsNetworkAgent" + index, 
                            reBuildVMNetworkCapabilities(index), reBuildVMLinkProperties(index), reBuildVMScore(index), 
                            reBuildVMNetworkAgentConfig(index), null){
                public void unwanted(){
                    if (this == mSystemAgent) {
                        //mConnectivityManager.unregisterNetworkProvider(mNetworkProvider);
                    } else if (mSystemAgent != null) {
                        Log.d(LOG_TAG, "Ignoring unwanted as we have a more modern " +
                                "instance");
                    }   // Otherwise, we've already called stop.
                    Log.d(LOG_TAG, "CellsNetworkAgent unwanted.");
                };
            };
            Log.d(LOG_TAG, "mLinkProperties=" + mLinkProperties.toString());
            Log.d(LOG_TAG, "mNetworkCapabilities=" + mNetworkCapabilities.toString());

            mSystemAgent.register();
            mSystemAgent.markConnected();
        }

        private void updateVMLinkProperties(){
            Thread vm = new Thread(new Runnable(){
                @Override
                public void run() {
                    do
                    {
                        SystemClock.sleep(6000);

                        IBinder ib = ServiceManager.getInitService(Context.CELLS_SERVICE);
                        if(ib != null){
                            ICellsService iservice = ICellsService.Stub.asInterface(ib);

                            NetworkInfo initNetworkInfo = null;
                            try {
                                initNetworkInfo = iservice.getActiveNetworkInfo();
                            } catch (RemoteException e) {
                                Log.e(LOG_TAG, "Couldn't getActiveNetworkInfo: " + e.toString());
                            }

                            if(initNetworkInfo == null || 
                               initNetworkInfo.getType() != ConnectivityManager.TYPE_WIFI){
                                if(mSystemAgent != null){
                                    mSystemAgent.unregister();
                                    mSystemAgent = null;
                                    Log.d(LOG_TAG, "mSystemAgent unregister");
                                }

                                continue;
                            }

                            if(mSystemAgent == null){
                                createVMNetworkAgent(mIndex);
                                if(mSystemAgent != null)
                                    Log.d(LOG_TAG, "mSystemAgent register");
                            }

                            LinkProperties initLinkProperties = null;
                            try {
                                initLinkProperties = iservice.getActiveLinkProperties();
                            } catch (RemoteException e) {
                                Log.e(LOG_TAG, "Couldn't getActiveLinkProperties: " + e.toString());
                            }

                            LinkProperties linkProperties = mCells.getActiveLinkProperties();

                            boolean noneedUpdate = initLinkProperties == null;
                            if(initLinkProperties != null && linkProperties != null)
                            {
                                List<InetAddress> inetAddress = linkProperties.getDnsServers();
                                List<InetAddress> initInetAddress = initLinkProperties.getDnsServers();

                                if(inetAddress.size() == initInetAddress.size())
                                {
                                    boolean bsame = true;
                                    for (int i=0; i < initInetAddress.size(); i++) {
                                        //if (initInetAddress.get(i) instanceof Inet4Address) 
                                        {
                                            if(!inetAddress.get(i).toString().equals(initInetAddress.get(i).toString())){
                                                bsame = false;
                                                Log.d(LOG_TAG, "updateVMLinkProperties: " + inetAddress.get(i).toString() + " != " + initInetAddress.get(i).toString());
                                                break;
                                            }
                                        }
                                    }
                                    noneedUpdate = bsame;
                                }
                            }

                            if(!noneedUpdate)
                            {
                                if(mSystemAgent != null){
                                    Log.d(LOG_TAG, "sendLinkProperties: update!");
                                    mSystemAgent.sendLinkProperties(reBuildVMLinkProperties(mIndex));
                                }
                            }
                        }
                    }while(true);
                }
            });
            vm.setDefaultUncaughtExceptionHandler(null);
            vm.start();
        }

        public CellsNetworkAgent(Context context, CellsService cells) {
            mContext = context;
            mCells = cells;
            String vmname = SystemProperties.get("ro.boot.vm.name","");
            if(vmname == null || vmname.length() <= 4){
                return ;
            }

            Matcher m = Pattern.compile("\\d+").matcher(vmname);
            if(m.find()){
                mIndex = Integer.parseInt(m.group());
                if(mIndex <= 0) return;
            }

            mConnectivityManager = mContext.getSystemService(ConnectivityManager.class);

            Thread vm = new Thread(new Runnable(){
                @Override
                public void run() {
                    Looper.prepare();
                    Log.d(LOG_TAG, "mSystemAgentHandler : loop!");
                    mSystemAgentHandler = new Handler(Looper.myLooper());
                    Looper.loop();
                    Log.d(LOG_TAG, "mSystemAgentHandler : quit!");
                }
            });
            vm.setDefaultUncaughtExceptionHandler(null);
            vm.start();

            updateVMLinkProperties();
        }
    }
}
