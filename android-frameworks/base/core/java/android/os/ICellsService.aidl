package android.os;

import android.net.LinkProperties;
import android.net.NetworkInfo;

/**
 * {@hide}
 */
interface ICellsService {
    boolean isSystemReady();
    LinkProperties getActiveLinkProperties();
    NetworkInfo getActiveNetworkInfo();
}
