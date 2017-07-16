package fr.mathieu.berengere.safdemo;
        import android.app.Dialog;
import android.content.Intent;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.net.Uri;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.Environment;
import android.support.v4.content.ContextCompat;
import android.support.v7.app.AppCompatActivity;
import android.util.DisplayMetrics;
import android.view.Display;
import android.view.MotionEvent;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.ImageButton;
import android.widget.LinearLayout;
import android.widget.TextView;

import java.io.FileOutputStream;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Calendar;


public class InteractiveSegmentationActivity extends AppCompatActivity {

    // Used to load the 'native-lib' library on application startup.
    static {
        System.loadLibrary("native-lib");
    }

    String imagePath;
    boolean superpixelsReady;
    //seeds layer
    private DrawingView seeds;
    //paint color
    private ImageButton currPaint;
    //paint brush
    private ImageButton drawBtn;
    //eraser button
    private ImageButton eraseBtn;
    //segment image using seeds
    private Button segmentationBtn;
    //send image and segmentation result
    private Button sendBtn;
    //move on image
    private ImageButton upButton, downButton, rightButton, leftButton;
    //brush sizes
    private float smallBrush, mediumBrush, largeBrush;
    private String spotLocation;

    /**
     * A native method that is implemented by the 'native-lib' native library,
     * which is packaged with this application.
     */
    private native int oversegmentWithSLIC(Bitmap image);

    private native int checkNumberOfClasses(Bitmap image);

    private native int interactiveSegmentation(Bitmap seeds,Bitmap segmentation);

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_interactive_segmentation);

        Intent intent = getIntent();
        imagePath=intent.getStringExtra(CameraActivity.PARAMETER_IMG_PATH);
        spotLocation=intent.getStringExtra(CameraActivity.PARAMETER_LOCATION);

        initializeLayers();
        //get color palett layout
        LinearLayout paintLayout = (LinearLayout)findViewById(R.id.paint_colors);
        //get first color
        currPaint = (ImageButton)paintLayout.getChildAt(0);
        //show that it currently selected
        currPaint.setImageDrawable(ContextCompat.getDrawable(getApplicationContext(),R.drawable.paint_pressed));


        //instantiate brush sizes
        smallBrush = getResources().getInteger(R.integer.small_size);
        mediumBrush = getResources().getInteger(R.integer.medium_size);
        largeBrush = getResources().getInteger(R.integer.large_size);
        seeds.setBrushSize(mediumBrush);

        //add paint button listener for brush size
        drawBtn = (ImageButton)findViewById(R.id.draw_btn);
        drawBtn.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                changeBrushSize();
            }
        });

        //add eraser button listener
        eraseBtn = (ImageButton)findViewById(R.id.erase_btn);
        eraseBtn.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                setEraser();
            }
        });

        upButton = (ImageButton)findViewById(R.id.upZoomButton);
        upButton.setOnTouchListener(new View.OnTouchListener() {
            @Override
            public boolean onTouch(View arg0, MotionEvent arg1) {
                seeds.moveUpZoom();
                return false;
            }
        });

        downButton = (ImageButton)findViewById(R.id.downZoomButton);
        downButton.setOnTouchListener(new View.OnTouchListener() {
            @Override
            public boolean onTouch(View arg0, MotionEvent arg1) {
                seeds.moveDownZoom();
                return false;
            }
        });
        rightButton = (ImageButton)findViewById(R.id.rightZoomButton);
        rightButton.setOnTouchListener(new View.OnTouchListener() {
            @Override
            public boolean onTouch(View arg0, MotionEvent arg1) {
                seeds.moveRightZoom();
                return false;
            }
        });
        leftButton = (ImageButton)findViewById(R.id.leftZoomButton);
        leftButton.setOnTouchListener(new View.OnTouchListener() {
            @Override
            public boolean onTouch(View arg0, MotionEvent arg1) {
                seeds.moveLeftZoom();
                return false;
            }
        });

        segmentationBtn = (Button)findViewById(R.id.segmentButton);

        segmentationBtn.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                TextView infoTextView=(TextView)findViewById(R.id.textViewInfo);
                infoTextView.setText(R.string.wait);

                segmentationBtn.setVisibility(View.GONE);
                sendBtn.setVisibility(View.GONE);

                int nbClasses=checkNumberOfClasses(seeds.getSeeds());
                if(nbClasses>=2) {
                    new SegmentationTask().execute(seeds.getSeeds());
                }else{
                    infoTextView.setText(R.string.errorNbColors);
                    segmentationBtn.setVisibility(View.VISIBLE);

                }

            }
        });

        sendBtn = (Button) findViewById(R.id.sendBoutton);
        sendBtn.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                sendEmail();

            }
        });
        sendBtn.setVisibility(View.GONE);




    }

    private void sendEmail() {

        //save original image
        FileOutputStream out = null;
        try {
            out = new FileOutputStream(imagePath);
            seeds.getImage().compress(Bitmap.CompressFormat.PNG, 100, out); // bmp is your Bitmap instance
            // PNG is a lossless format, the compression factor (100) is ignored
        } catch (Exception e) {
            e.printStackTrace();
        } finally {
            try {
                if (out != null) {
                    out.close();
                }
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
        //save segmentation image
        String segmentationPath= Environment.getExternalStorageDirectory().toString();
        segmentationPath=segmentationPath+"/SAF/segmentation.png";

        out = null;
        try {
            out = new FileOutputStream(segmentationPath);
            seeds.getSegmentation().compress(Bitmap.CompressFormat.PNG, 100, out); // bmp is your Bitmap instance
            // PNG is a lossless format, the compression factor (100) is ignored
        } catch (Exception e) {
            e.printStackTrace();
        } finally {
            try {
                if (out != null) {
                    out.close();
                }
            } catch (IOException e) {
                e.printStackTrace();
            }
        }

        //get admin mail
        String mail="berengere.ma@gmail.com";
        if(mail !=null &&mail.length()>0){
            //send email with original image and segmentation
            String[] TO = {mail};
            Intent emailIntent = new Intent(Intent.ACTION_SEND_MULTIPLE);

            emailIntent.setData(Uri.parse("mailto:"));
            emailIntent.setType("text/plain");
            emailIntent.putExtra(Intent.EXTRA_EMAIL, TO);
            emailIntent.putExtra(Intent.EXTRA_SUBJECT, "Demo Saf : result");
            //get date
            String mydate = java.text.DateFormat.getDateTimeInstance().format(Calendar.getInstance().getTime());
            String text="Date: "+mydate+"\nLocation: "+spotLocation;
            emailIntent.putExtra(Intent.EXTRA_TEXT,text);

            ArrayList<Uri> emailsData = new ArrayList<Uri>();
            emailsData.add(Uri.parse("file://"+segmentationPath));
            emailsData.add( Uri.parse("file://"+imagePath));
            emailIntent.putParcelableArrayListExtra(Intent.EXTRA_STREAM,emailsData);


            try {
                startActivity(Intent.createChooser(emailIntent, "Send mail..."));
                finish();
            } catch (android.content.ActivityNotFoundException ex) {
                TextView infoTextView=(TextView)findViewById(R.id.textViewInfo);
                infoTextView.setText(R.string.errorMailClient);
            }
        }


    }

    private void setEraser(){
        if(seeds.getEraser()){
            //stop to erase
            eraseBtn.setImageBitmap(BitmapFactory.decodeResource(getResources(), R.drawable.eraser));
            seeds.setErase(false);

        }else {

            eraseBtn.setImageBitmap(BitmapFactory.decodeResource(getResources(), R.drawable.brush));
            seeds.setErase(true);
        }
    }

    private void changeBrushSize(){
        //display a dialog repreasenting brush size
        final Dialog brushDialog = new Dialog(this);
        brushDialog.setTitle("Brush size:");
        brushDialog.setContentView(R.layout.brush_chooser);
        ImageButton smallBtn = (ImageButton)brushDialog.findViewById(R.id.small_brush);
        smallBtn.setOnClickListener(new OnClickListener(){
            @Override
            public void onClick(View v) {
                seeds.setBrushSize(smallBrush);
                seeds.setLastBrushSize(smallBrush);
                brushDialog.dismiss();
            }
        });
        ImageButton mediumBtn = (ImageButton)brushDialog.findViewById(R.id.medium_brush);
        mediumBtn.setOnClickListener(new OnClickListener(){
            @Override
            public void onClick(View v) {
                seeds.setBrushSize(mediumBrush);
                seeds.setLastBrushSize(mediumBrush);
                brushDialog.dismiss();
            }
        });

        ImageButton largeBtn = (ImageButton)brushDialog.findViewById(R.id.large_brush);
        largeBtn.setOnClickListener(new OnClickListener(){
            @Override
            public void onClick(View v) {
                seeds.setBrushSize(largeBrush);
                seeds.setLastBrushSize(largeBrush);
                brushDialog.dismiss();
            }
        });
        brushDialog.show();

    }

    private void initializeLayers() {
        seeds=(DrawingView) findViewById(R.id.imageViewSeeds);
        WindowManager w = getWindowManager();
        Display d = w.getDefaultDisplay();
        DisplayMetrics metrics = new DisplayMetrics();
        d.getMetrics(metrics);
        seeds.loadImage(imagePath,metrics.widthPixels);
        new OversegmentationTask().execute(seeds.getImage());
    }

    private void drawSegmentationLayer(){

    }

    public void paintClicked(View view){
        //use chosen color
        if(view!=currPaint){
            //update color
            ImageButton imgView = (ImageButton)view;
            String color = view.getTag().toString();
            //change seeds color
            seeds.setColor(color);
            //update UI
            imgView.setImageDrawable(ContextCompat.getDrawable(getApplicationContext(),R.drawable.paint_pressed));
            currPaint.setImageDrawable(ContextCompat.getDrawable(getApplicationContext(),R.drawable.paint));
            currPaint=(ImageButton)view;
        }
    }

    private class SegmentationTask extends AsyncTask<Bitmap, Void, Bitmap> {
        private boolean hasTwoClasses;
        @Override
        protected Bitmap doInBackground(Bitmap... seeds) {
            Bitmap   segmentation= Bitmap.createBitmap(seeds[0].getWidth(),
                    seeds[0].getHeight(), Bitmap.Config.ARGB_8888);
            while(!superpixelsReady){
                try {
                    Thread.sleep(500);
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
            }
            hasTwoClasses=true;
            interactiveSegmentation(seeds[0],segmentation);
            return segmentation;
        }

        @Override
        protected void onPostExecute(Bitmap res) {
            if(hasTwoClasses){
                seeds.setSegmentationLayer(res);
                sendBtn.setVisibility(View.VISIBLE);
                TextView infoTextView=(TextView)findViewById(R.id.textViewInfo);
                infoTextView.setText(R.string.segmentationDone);
            }else{
                TextView infoTextView=(TextView)findViewById(R.id.textViewInfo);
                infoTextView.setText(R.string.errorNbColors);
            }

            segmentationBtn.setVisibility(View.VISIBLE);

        }

        @Override
        protected void onPreExecute() {
        }

        @Override
        protected void onProgressUpdate(Void... values) {
        }
    }

    private class OversegmentationTask extends AsyncTask<Bitmap, Void, Integer> {
        @Override
        protected Integer doInBackground(Bitmap ... image ) {
            superpixelsReady=false;

            return oversegmentWithSLIC(image[0]);
        }

        @Override
        protected void onPostExecute(Integer nbSps) {
            superpixelsReady=true;
        }

        @Override
        protected void onPreExecute() {
        }

        @Override
        protected void onProgressUpdate(Void... values) {
        }
    }
}

