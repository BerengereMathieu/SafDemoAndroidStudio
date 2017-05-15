package fr.mathieu.berengere.safdemo;

import android.Manifest;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.location.Location;
import android.location.LocationManager;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.net.Uri;
import android.os.Bundle;
import android.os.Environment;
import android.support.v4.app.ActivityCompat;
import android.support.v7.app.AppCompatActivity;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;

import com.google.android.gms.appindexing.Action;
import com.google.android.gms.appindexing.AppIndex;
import com.google.android.gms.appindexing.Thing;
import com.google.android.gms.common.api.GoogleApiClient;

import java.io.File;
public class LocationActivity extends AppCompatActivity {

    public final static String PARAMETER_SPOT_LOCATION = "fr.mathieu.berengere.safdemo.GetGuidLocation";

    private Double latitude;
    private Double longitude;

    /**
     * ATTENTION: This was auto-generated to implement the App Indexing API.
     * See https://g.co/AppIndexing/AndroidStudio for more information.
     */
    private GoogleApiClient client;

    private void checkConnection() {
        //check connection to download spot image
        ConnectivityManager cm = (ConnectivityManager) getApplicationContext().getSystemService(Context.CONNECTIVITY_SERVICE);
        NetworkInfo activeNetwork = cm.getActiveNetworkInfo();
        if ((activeNetwork != null) &&
                ((activeNetwork.getType() == ConnectivityManager.TYPE_WIFI) ||
                        (activeNetwork.getType() == ConnectivityManager.TYPE_MOBILE))) {
            //connection is available
            //try to get location
            Button nextButton = (Button) findViewById(R.id.button);
            nextButton.setOnClickListener(new View.OnClickListener() {
                                              public void onClick(View v) {
                                                  tryToGetLocation();
                                              }
                                          }
            );
            TextView informationTextView = (TextView) findViewById(R.id.textViewInfo);
            informationTextView.setText("Unable to get a location.");
            tryToGetLocation();
        }
    }

    private void tryToGetLocation() {
        //ask for a connection


        boolean hasLocation = false;
        //get textview to get information to user
        TextView informationTextView = (TextView) findViewById(R.id.textViewInfo);
        if (ActivityCompat.checkSelfPermission(this, Manifest.permission.ACCESS_FINE_LOCATION) != PackageManager.PERMISSION_GRANTED && ActivityCompat.checkSelfPermission(this, Manifest.permission.ACCESS_COARSE_LOCATION) != PackageManager.PERMISSION_GRANTED) {
            informationTextView.setText("No permission to use fine and coarse location.");
        } else {
            //get location manadger
            LocationManager locationManager = (LocationManager) getApplicationContext().getSystemService(Context.LOCATION_SERVICE);

            //first try to use network location provider
            if (!locationManager.isProviderEnabled(LocationManager.NETWORK_PROVIDER)) {
                //try to use passive location
                if (!locationManager.isProviderEnabled(LocationManager.PASSIVE_PROVIDER)) {
                    //try to use GPS location provider
                    if (!locationManager.isProviderEnabled(LocationManager.GPS_PROVIDER)) {
                        //ask user to activate GPS or Network
                        informationTextView.setText("No location provider available. ");
                    } else {
                        //use GPs location provider
                        //search location using GPS
                        Location location = locationManager.getLastKnownLocation(LocationManager.GPS_PROVIDER);
                        if (location != null) {
                            latitude = location.getLatitude();
                            longitude = location.getLongitude();
                            hasLocation = true;
                        }
                    }
                } else {
                    //search location using passive location provider
                    Location location = locationManager.getLastKnownLocation(LocationManager.PASSIVE_PROVIDER);
                    if (location != null) {
                        latitude = location.getLatitude();
                        longitude = location.getLongitude();
                        hasLocation = true;
                    }

                }
            } else {
                //search location using gps
                Location location = locationManager.getLastKnownLocation(LocationManager.NETWORK_PROVIDER);
                if (location != null) {
                    latitude = location.getLatitude();
                    longitude = location.getLongitude();
                    hasLocation = true;
                }
            }
        }
        if (hasLocation) {
            //display location
            TextView locationTextView=(TextView) findViewById(R.id.locationTextView);
            locationTextView.setText("GPS coordinates : " + latitude +" , " + longitude);
            //reset button
            Button button=(Button) findViewById(R.id.button);
            button.setText("Next");
            button.setOnClickListener(
                    new View.OnClickListener() {
                        public void onClick(View v) {
                            startTakePictureActivity();
                        }
                    }

            );
            //give indication
            TextView textViewTag=(TextView) findViewById(R.id.textViewTag);
            textViewTag.setText("Ready");
            TextView textViewInfo=(TextView) findViewById(R.id.textViewInfo);
            textViewInfo.setText("Now take a picture");
        }
    }

    private void startTakePictureActivity() {
        Intent intent = new Intent(this, TakePictureActivity.class);
        String location = "(" + latitude.toString() + "," + longitude.toString() + ")";
        intent.putExtra(PARAMETER_SPOT_LOCATION, location);
        startActivity(intent);


    }



    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_location);
        Button tryToConnectButton = (Button) findViewById(R.id.button);
        tryToConnectButton.setOnClickListener(new View.OnClickListener() {
                                                  public void onClick(View v) {
                                                      checkConnection();
                                                  }
                                              }
        );
        checkConnection();

        //create application directory if not exist
        // create a File object for the parent directory
        File appDirectory = new File(Environment.getExternalStorageDirectory().toString() + "/SAF/");
        // have the object build the directory structure, if needed.
        appDirectory.mkdirs();


        // ATTENTION: This was auto-generated to implement the App Indexing API.
        // See https://g.co/AppIndexing/AndroidStudio for more information.
        client = new GoogleApiClient.Builder(this).addApi(AppIndex.API).build();


    }


    /**
     * ATTENTION: This was auto-generated to implement the App Indexing API.
     * See https://g.co/AppIndexing/AndroidStudio for more information.
     */
    public Action getIndexApiAction() {
        Thing object = new Thing.Builder()
                .setName("GetImage Page")
                .setUrl(Uri.parse("http://[ENTER-YOUR-URL-HERE]"))
                .build();
        return new Action.Builder(Action.TYPE_VIEW)
                .setObject(object)
                .setActionStatus(Action.STATUS_TYPE_COMPLETED)
                .build();
    }

    @Override
    public void onStart() {
        super.onStart();
        client.connect();
        AppIndex.AppIndexApi.start(client, getIndexApiAction());
    }

    @Override
    public void onStop() {
        super.onStop();
        AppIndex.AppIndexApi.end(client, getIndexApiAction());
        client.disconnect();
    }

}
