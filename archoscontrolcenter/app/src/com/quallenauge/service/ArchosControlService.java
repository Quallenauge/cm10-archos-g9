package com.quallenauge.service;

import java.io.BufferedReader;
import java.io.FileReader;
import java.io.InputStreamReader;

import android.app.Service;
import android.appwidget.AppWidgetManager;
import android.content.ComponentName;
import android.content.Intent;
import android.os.IBinder;
import android.util.Log;
import android.widget.RemoteViews;
import android.widget.Toast;

import com.quallenauge.widget.ArchosControlWidget;
import com.quallenauge.widget.R;

public class ArchosControlService extends Service {
	public static final String EXTRA_MODE		 	 = "EXTRA_MODE";
	public static final String EXTRA_MODE_UPDATE 	 = "EXTRA_MODE_UPDATE";
	public static final String EXTRA_MODE_3G_PORT 	 = "EXTRA_MODE_3G_PORT";
	public static final String EXTRA_MODE_DEEP_SLEEP = "EXTRA_MODE_DEEP_SLEEP";
	
	public static final String EXTRA_MODE_VALUE	 	 = "EXTRA_MODE_VALUE";	
	
	private static final String TAG = "ArchosControlService";
	private static final String DEEP_SLEEP_BINARY  = "/system/bin/deepsleeper";
    private static final String SYSFS_RFKILL_ENTRY = "/sys/class/rfkill/rfkill0/state";
	
	private boolean m_3GPortMode = false;
	private boolean m_DeepSleepMode = false;
	
	@Override
	public IBinder onBind(Intent arg0) {
		return null;
	}

	@Override
	public int onStartCommand(Intent intent, int flags, int startId) {
		Log.v(TAG, "Service is started..."+flags);
		super.onStartCommand(intent, flags, startId);
		
		m_3GPortMode = get3GPortMode();
		Log.v(TAG, "3G Port state <"+get3GPortMode()+">.");
		
		if (ArchosControlService.EXTRA_MODE_3G_PORT.equals( intent.getExtras().get(ArchosControlService.EXTRA_MODE))){
			if (m_3GPortMode=set3GPortMode(!m_3GPortMode)){
				Log.v(TAG, "3G-Port switch from <"+!m_3GPortMode+"> to <"+m_3GPortMode+"> requested!");
			}
		}else if (ArchosControlService.EXTRA_MODE_DEEP_SLEEP.equals( intent.getExtras().get(ArchosControlService.EXTRA_MODE))){
			if (setDeepSleep( !m_DeepSleepMode )){
				Log.v(TAG, "Deep sleep mode switch from <"+m_DeepSleepMode+"> to <"+!m_DeepSleepMode+"> requested!");
				m_DeepSleepMode = !m_DeepSleepMode;
			}
		}else if (ArchosControlService.EXTRA_MODE_UPDATE.equals( intent.getExtras().get(ArchosControlService.EXTRA_MODE))){
			Log.v(TAG, "Update of widgets requested...");
		}else{
			Log.v(TAG, "Error! Intent type not supported! - Updating widgets.");
		}
		
		RemoteViews remoteViews = new RemoteViews(this.getApplicationContext().getPackageName(), R.layout.main);
		remoteViews.setTextViewText(R.id.tv3GPortState, m_3GPortMode ? "On": "Off");
		remoteViews.setTextViewText(R.id.tvDeepSleepState,m_DeepSleepMode ? "On" : "Off");	      
		
		ComponentName thiswidget = new ComponentName(this.getApplicationContext(), ArchosControlWidget.class);
		AppWidgetManager manager = AppWidgetManager.getInstance(this.getApplicationContext());
		manager.updateAppWidget(thiswidget, remoteViews);
		
		// do the action here
		return (START_STICKY);
	}
	
	private boolean setDeepSleep(boolean state){
		try{
			Process process = Runtime.getRuntime().exec(DEEP_SLEEP_BINARY+" 1");
			final BufferedReader stdErr = new BufferedReader(new InputStreamReader(process.getErrorStream()));
			
			if (process.waitFor()==0){
				Toast.makeText(getApplicationContext(), (state?"Enabled":"Disabled")+" deep sleep.!", Toast.LENGTH_LONG).show();
			}else{
	            String line;
	            try {
	                while ((line = stdErr.readLine()) != null) {
	                	Log.e(TAG, "DeepSleep binary: "+line);
	                }
	            } catch (Exception e) {throw new Error(e);}
				Toast.makeText(getApplicationContext(), "Can't set deep sleep state!", Toast.LENGTH_LONG).show();
				return false;
			}
			Log.e(TAG, "Result code was: <"+process.exitValue()+">");
		}catch(Exception p_Ex){
			Log.e(TAG, "Error! Can't execute deepsleeper binary.", p_Ex);
			Toast.makeText(getApplicationContext(), "Can't set deep sleep state (Exception)!", Toast.LENGTH_LONG).show();
			return false;
		}
		return true;
	}
	
	private boolean set3GPortMode(boolean state){
		   /*
		    *  Switch off the ril daemon. It is restarted when hotplug service sees a new device.
		    */
		   SystemPropertiesProxy.set(this.getApplicationContext(), "ctl.stop", "ril-daemon");
		   
//       try {
//         BufferedWriter bw = new BufferedWriter(new FileWriter(SYSFS_RFKILL_ENTRY));
//         bw.write(state ? "1" : "0");
//         bw.close();
//        } catch (Exception p_Ex) {
//        	Log.e(TAG, "Error! Can't set 3G Port mode on file <"+SYSFS_RFKILL_ENTRY+">.", p_Ex);
//			Toast.makeText(getApplicationContext(), "Can't set 3G Port mode (Exception)!", Toast.LENGTH_LONG).show();		            	
//        	return false;
//        }
	   if (state){
		   SystemPropertiesProxy.set(this.getApplicationContext(), "ctl.start", "set3GPortState:1");
	   }else{
		   SystemPropertiesProxy.set(this.getApplicationContext(), "ctl.start", "set3GPortState:0");
	   }
       Toast.makeText(getApplicationContext(), (state?"Enabled":"Disabled")+" 3G Port mode!", Toast.LENGTH_LONG).show();
       return state;
    }
	
	private boolean get3GPortMode(){
		boolean state=false;
	       try { 
	         BufferedReader bw = new BufferedReader(new FileReader(SYSFS_RFKILL_ENTRY));
	         String line = bw.readLine();
	         bw.close();
	         state = Integer.parseInt(line)==1?true:false;
	        } catch (Exception p_Ex) {
	        	Log.e(TAG, "Error! Can't get 3G Port mode on file <"+SYSFS_RFKILL_ENTRY+">.", p_Ex);
				Toast.makeText(getApplicationContext(), "Can't get 3G Port mode (Exception)!", Toast.LENGTH_LONG).show();		            	
	        	return false;
	        }
	       return state;
	    }
}
