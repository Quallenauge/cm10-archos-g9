package com.quallenauge.widget;

import android.app.PendingIntent;
import android.appwidget.AppWidgetManager;
import android.appwidget.AppWidgetProvider;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.util.Log;
import android.widget.RemoteViews;

import com.quallenauge.service.ArchosControlService;

public class ArchosControlWidget extends AppWidgetProvider {
	private static final String TAG = "ArchosControlWidget";
	
	@Override
	public void onUpdate(Context context, AppWidgetManager appWidgetManager, int[] appWidgetIds) {
		Log.v(TAG, "onUpdate...");
        final int N = appWidgetIds.length;

        
        ComponentName thisWidget = new ComponentName(context,ArchosControlWidget.class);
        int[] allWidgetIds = appWidgetManager.getAppWidgetIds(thisWidget);
        
        // Perform this loop procedure for each App Widget that belongs to this provider
        for (int i=0; i<N; i++) {
        	int appWidgetId = appWidgetIds[i];
        	
        	Log.v(TAG, "Install listeners for widget <"+appWidgetId+">...");	
        	
			// Create an Intent to launch ExampleActivity
	        Intent intent = new Intent(context, ArchosControlService.class);
	        intent.putExtra(ArchosControlService.EXTRA_MODE, ArchosControlService.EXTRA_MODE_DEEP_SLEEP);
	        intent.putExtra(ArchosControlService.EXTRA_MODE_VALUE, false);
	        PendingIntent pendingIntent = PendingIntent.getService(context, 1, intent, PendingIntent.FLAG_UPDATE_CURRENT);
	        
	        // Get the layout for the App Widget and attach an on-click listener to the button
	        RemoteViews views = new RemoteViews(context.getPackageName(), R.layout.main);
	        views.setOnClickPendingIntent(R.id.btnDeepSleep, pendingIntent);
	        
	        intent = new Intent(context, ArchosControlService.class);
	        intent.putExtra(ArchosControlService.EXTRA_MODE, ArchosControlService.EXTRA_MODE_3G_PORT);
	        intent.putExtra(ArchosControlService.EXTRA_MODE_VALUE, false);
	        intent.putExtra(AppWidgetManager.EXTRA_APPWIDGET_IDS, allWidgetIds);
	        
	        pendingIntent = PendingIntent.getService(context, 2, intent, PendingIntent.FLAG_UPDATE_CURRENT);
	        views.setOnClickPendingIntent(R.id.btn3GPortSwitch, pendingIntent);
	
	        // Tell the AppWidgetManager to perform an update on the current App Widget
	        appWidgetManager.updateAppWidget(appWidgetId, views);
        }
        
        Intent intent = new Intent(context, ArchosControlService.class);
        intent.putExtra(ArchosControlService.EXTRA_MODE, ArchosControlService.EXTRA_MODE_UPDATE);
        context.startService(intent);
        
	}
	   @Override
	    public void onReceive(Context context, Intent intent) {
	        super.onReceive(context, intent);
	        Log.v(TAG, "onReceive..."+intent.getAction()+"|"+intent.toString());
	   }

	
}
