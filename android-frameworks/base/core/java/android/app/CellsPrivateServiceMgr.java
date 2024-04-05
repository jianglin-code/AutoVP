package android.app;

import android.content.Context;
import android.content.Intent;
import android.os.Binder;
import android.os.Bundle;
import android.os.IBinder;
import android.os.Parcel;
import android.os.Parcelable;
import android.os.PooledStringReader;
import android.os.PooledStringWriter;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.os.UserHandle;
import android.util.Log;

import android.app.IActivityManager;
import android.app.Activity;
import android.app.AppOpsManager;

import android.os.SystemProperties;

import android.annotation.NonNull;
import android.annotation.Nullable;

import java.util.regex.Matcher;
import java.util.regex.Pattern;

public class CellsPrivateServiceMgr {
    static String TAG = "CellsPrivateServiceMgr";
    static final int  SETPROPERTY = Binder.FIRST_CALL_TRANSACTION;
    static final int  STARTCELLSVM = Binder.FIRST_CALL_TRANSACTION + 1;
    static final int  STOPCELLSVM = Binder.FIRST_CALL_TRANSACTION + 2;
    static final int  SWITCHCELLSVM = Binder.FIRST_CALL_TRANSACTION + 3;
    static final int  UPLOADCELLSVM = Binder.FIRST_CALL_TRANSACTION + 4;
    static final int  DOWNLOADCELLSVM = Binder.FIRST_CALL_TRANSACTION + 5;
    static final int  UNTARCELLSVM = Binder.FIRST_CALL_TRANSACTION + 6;
    static final int  TARCELLSVM = Binder.FIRST_CALL_TRANSACTION + 7;
    static final int  SYSTEMREADY = Binder.FIRST_CALL_TRANSACTION + 8;
    static final int  SENDCELLSVM = Binder.FIRST_CALL_TRANSACTION + 15;

    static final int  MAX_CONTEXT = 2;
    private final Context mContext;
    private final IBinder mChannel;
    public CellsPrivateServiceMgr(@NonNull Context context, @NonNull IBinder channel ) {
        mContext = context; 
        mChannel = channel;
    }

    public CellsPrivateServiceMgr(@NonNull Context context) {
        mContext = context; 
        mChannel = ServiceManager.getService("CellsPrivateService");
    }

    public int getCurrentSystem() {
        int index = 0;
        String vmname = SystemProperties.get("persist.sys.active","");
        if(vmname == null || vmname.length() <= 4){
            return 0;
        }

        Matcher m = Pattern.compile("\\d+").matcher(vmname);
        if(m.find()){
            index = Integer.parseInt(m.group());
            if(index <= 0) return 0;
        }

        return index;
    }

    public void sendOtherSystemBroadcastAsUser(@Nullable Intent intent, @Nullable UserHandle user, 
        @Nullable String receiverPermission, boolean sticky, int index) {
        String resolvedType = intent.resolveTypeIfNeeded(mContext.getContentResolver());
        String[] receiverPermissions = receiverPermission == null ? null
                : new String[] {receiverPermission};
        try {
            intent.prepareToLeaveProcess(mContext);
            IBinder b = ServiceManager.getOtherSystemService("activity", index);
            if(b!=null){
                IActivityManager am = IActivityManager.Stub.asInterface(b);
                if(am!=null){
                    am.broadcastIntentWithFeature(
                            mContext.getIApplicationThread(), mContext.getAttributionTag(), intent, resolvedType,
                            null, Activity.RESULT_OK, null, null, receiverPermissions, null /*excludedPermissions=*/, null,
                            AppOpsManager.OP_NONE, null, false, sticky, user.getIdentifier());
                }else{
                    Log.d(TAG, "Unable to get  other system  service IActivityManager " + index);
                }
            }else{
                Log.d(TAG, "Unable to get  other system  service activity " + index);
            }
        } catch (RemoteException e) {
            throw e.rethrowFromSystemServer();
        }
    }

    public void sendCurrentSystemBroadcastAsUser(@Nullable Intent intent, @Nullable UserHandle user, 
        @Nullable String receiverPermission, boolean sticky) {
        int index = getCurrentSystem();
        Log.d(TAG, "Current System : " + index);
        if(index > 0 && index <= MAX_CONTEXT){
            sendOtherSystemBroadcastAsUser(intent, user, receiverPermission, sticky, index);
        }
    }

    public void sendOtherSystemsBroadcastAsUser(@Nullable Intent intent, @Nullable UserHandle user, 
        @Nullable String receiverPermission, boolean sticky) {
        for(int i = 1; i <= MAX_CONTEXT; i++)
        {
            sendOtherSystemBroadcastAsUser(intent, user, receiverPermission, sticky, i);
        }
    }

    public int startCellsVm(@Nullable String name){
        if(mChannel == null) return 0;

        int ret = 0;
        Parcel data = Parcel.obtain();
        Parcel reply = Parcel.obtain();
        try {
            Log.e(TAG,"startCellsVm start ");
            data.writeInterfaceToken("CellsPrivateService");
            data.writeString(name);
            mChannel.transact(STARTCELLSVM, data, reply, 0);
            reply.readException();
            Log.e(TAG,"startCellsVm end ");
            ret = reply.readInt();
        }catch(RemoteException e){
            e.printStackTrace();
        }finally {
            data.recycle();
            reply.recycle();
        }
        return ret;
    }

    public int stopCellsVm(@Nullable String name) {
        if(mChannel == null) return 0;

        int ret = 0;
        Parcel data = Parcel.obtain();
        Parcel reply = Parcel.obtain();
        try {
            Log.e(TAG,"stopCellsVm start ");
            data.writeInterfaceToken("CellsPrivateService");
            data.writeString(name);
            mChannel.transact(STOPCELLSVM, data, reply, 0);
            reply.readException();
            Log.e(TAG,"stopCellsVm end ");
        }catch(RemoteException e){
            e.printStackTrace();
        }finally {
            data.recycle();
            reply.recycle();
        }
        return ret;
    }

    public int switchCellsVm(@Nullable String name) {
        if(mChannel == null) return 0;

        int ret = 0;
        Parcel data = Parcel.obtain();
        Parcel reply = Parcel.obtain();
        try {
            Log.e(TAG,"switchCellsVm start ");
            data.writeInterfaceToken("CellsPrivateService");
            data.writeString(name);
            mChannel.transact(SWITCHCELLSVM, data, reply, 0);
            reply.readException();
            Log.e(TAG,"switchCellsVm end ");
        }catch(RemoteException e){
            e.printStackTrace();
        }finally {
            data.recycle();
            reply.recycle();
        }
        return ret;
    }

    public int uploadCellsVm(@Nullable String name) {
        if(mChannel == null) return 0;

        int ret = 0;
        Parcel data = Parcel.obtain();
        Parcel reply = Parcel.obtain();
        try {
            Log.e(TAG,"uploadCellsVm start ");
            data.writeInterfaceToken("CellsPrivateService");
            data.writeString(name);
            mChannel.transact(UPLOADCELLSVM, data, reply, 0);
            reply.readException();
            Log.e(TAG,"uploadCellsVm end ");
        }catch(RemoteException e){
            e.printStackTrace();
        }finally {
            data.recycle();
            reply.recycle();
        }
        return ret;
    }

    public int downloadCellsVm(@Nullable String name) {
        if(mChannel == null) return 0;

        int ret = 0;
        Parcel data = Parcel.obtain();
        Parcel reply = Parcel.obtain();
        try {
            Log.e(TAG,"downloadCellsVm start ");
            data.writeInterfaceToken("CellsPrivateService");
            data.writeString(name);
            mChannel.transact(DOWNLOADCELLSVM, data, reply, 0);
            reply.readException();
            Log.e(TAG,"downloadCellsVm end ");
        }catch(RemoteException e){
            e.printStackTrace();
        }finally {
            data.recycle();
            reply.recycle();
        }
        return ret;
    }

    public int untarCellsVm(@Nullable String name) {
        if(mChannel == null) return 0;

        int ret = 0;
        Parcel data = Parcel.obtain();
        Parcel reply = Parcel.obtain();
        try {
            Log.e(TAG,"untarCellsVm start ");
            data.writeInterfaceToken("CellsPrivateService");
            data.writeString(name);
            mChannel.transact(UNTARCELLSVM, data, reply, 0);
            reply.readException();
            Log.e(TAG,"untarCellsVm end ");
        }catch(RemoteException e){
            e.printStackTrace();
        }finally {
            data.recycle();
            reply.recycle();
        }
        return ret;
    }

    public int tarCellsVm(@Nullable String name) {
        if(mChannel == null) return 0;

        int ret = 0;
        Parcel data = Parcel.obtain();
        Parcel reply = Parcel.obtain();
        try {
            Log.e(TAG,"tarCellsVm start ");
            data.writeInterfaceToken("CellsPrivateService");
            data.writeString(name);
            mChannel.transact(TARCELLSVM, data, reply, 0);
            reply.readException();
            Log.e(TAG,"tarCellsVm end ");
        }catch(RemoteException e){
            e.printStackTrace();
        }finally {
            data.recycle();
            reply.recycle();
        }
        return ret;
    }

    public int sendCellsVm(@Nullable String path, @Nullable String address) {
        if(mChannel == null) return 0;

        int ret = 0;
        Parcel data = Parcel.obtain();
        Parcel reply = Parcel.obtain();
        try {
            Log.e(TAG,"sendCellsVm start ");
            data.writeInterfaceToken("CellsPrivateService");
            data.writeString(path);
            data.writeString(address);
            mChannel.transact(SENDCELLSVM, data, reply, 0);
            //reply.readException();
            Log.e(TAG,"sendCellsVm end ");
        }catch(RemoteException e){
            e.printStackTrace();
        }finally {
            data.recycle();
            reply.recycle();
        }
        return ret;
    }

    public int vmSystemReady() {
        if(mChannel == null) return 0;

        String vmname = SystemProperties.get("ro.boot.vm.name","");
        if(vmname == null || vmname.length() <= 4){
            return 0;
        }

        int ret = 0;
        Parcel data = Parcel.obtain();
        Parcel reply = Parcel.obtain();
        try {
            Log.e(TAG,"systemReady start ");
            data.writeInterfaceToken("CellsPrivateService");
            data.writeString(vmname);
            mChannel.transact(SYSTEMREADY, data, reply, 0);
            reply.readException();
            Log.e(TAG,"systemReady end ");
        }catch(RemoteException e){
            e.printStackTrace();
        }finally {
            data.recycle();
            reply.recycle();
        }
        return ret;
    }

}
