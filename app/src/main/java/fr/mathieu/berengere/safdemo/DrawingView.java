package fr.mathieu.berengere.safdemo;


import android.content.Context;
import android.content.res.Resources;
import android.graphics.BitmapFactory;
import android.graphics.Color;
import android.graphics.Rect;
import android.util.AttributeSet;

import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.graphics.Path;
import android.view.MotionEvent;
import android.view.View;

import android.graphics.PorterDuff;
import android.graphics.PorterDuffXfermode;
import android.util.TypedValue;

/**
 * Created by berengere on 30/12/16.
 */

public class DrawingView extends View {
    //drawing path
    private Path seedsPath;
    //drawing and canvas paint
    private Paint drawPaint, canvasPaint;
    //initial color
    private int paintColor = 0xFFFFFFFF;
    //canvas
    private Canvas seedsCanvas;
    //image to segment
    private Bitmap imageBitmap;
    //seeds bitmap
    private Bitmap seedsBitmap;
    //segmentation bitmap
    private Bitmap segmentationBitmap;


    //area to display the all image with seeds
    private Rect miniatureArea;
    //image rectangle
    private Rect imageRect;
    //area to display the magnified image part
    private Rect zoomArea;
    //the magnified image part
    private Rect partImageArea;
    private int x0Zoom, y0Zoom;
    private int padding;

    //to load image only when component size is known.
    String imagePath;

    //to avoid drawing outside the drawing area
    int state;

    //current brush size
    private float brushSize;
    //to keep track of the last brush size used when the user switches to the eraser
    private float lastBrushSize;

    //ereaser usage
    private boolean erase=false;

    private int moveStep=10;

    private int maxImageWidth =1024;


    public DrawingView(Context context, AttributeSet attrs){
        super(context, attrs);
        setupDrawing();
        padding=10;
        state=0;
    }


    public void loadImage(String imagePath,int displayWidth){
        this.imagePath=imagePath;
        rescaleImage(displayWidth);

    }

    int rescaleDim(int originalA,int originalB,int wantedA){
        float ratio = (float) originalA / wantedA;
        return (int)(originalB / ratio);
    }

    private void setupDrawing(){
        //create tools to draw
        seedsPath = new Path();
        drawPaint = new Paint();
        //set the initial color
        drawPaint.setColor(paintColor);
        //set the initial path properties
        drawPaint.setAntiAlias(false);
        //set brush size
        brushSize = getResources().getInteger(R.integer.medium_size);
        lastBrushSize = brushSize;
        drawPaint.setStrokeWidth(brushSize);
        drawPaint.setStyle(Paint.Style.STROKE);
        drawPaint.setStrokeJoin(Paint.Join.ROUND);
        drawPaint.setStrokeCap(Paint.Cap.ROUND);
        //instantiate the canvas paint object
        canvasPaint = new Paint(Paint.DITHER_FLAG);

    }

    private void organizeScreen(int displayWidth,int displayHeight,int imageWidth,int imageHeight){
        //set padding
        padding=10;
        //set miniature area
        int heightMiniatureArea=displayHeight/3;
        int widthMiniatureArea=rescaleDim(imageHeight,imageWidth,heightMiniatureArea);
        int x0=(displayWidth-widthMiniatureArea)/2;
        miniatureArea=new Rect(x0,0,widthMiniatureArea+x0,heightMiniatureArea);
        //set area for up button
        int zoomAreaWidth=displayWidth;
        int y0=2*padding+heightMiniatureArea;
        int zoomAreaHeight=displayHeight-y0;
        zoomArea=new Rect(0,y0,zoomAreaWidth,y0+zoomAreaHeight);

        x0Zoom=0;
        y0Zoom=0;


    }

    public static int dpToPx(int dp)
    {
        return (int) (dp * Resources.getSystem().getDisplayMetrics().density);
    }

    private void rescaleImage(int displayWidth){
        maxImageWidth=Math.max(displayWidth,maxImageWidth);
        //load image
        Bitmap originalImage= BitmapFactory.decodeFile(imagePath);
        //rescale image for efficiency purpose
        int newHeight=rescaleDim(originalImage.getWidth(),originalImage.getHeight(), maxImageWidth);
        imageBitmap=Bitmap.createScaledBitmap(originalImage, maxImageWidth,newHeight,true);

    }
    @Override
    protected void onSizeChanged(int w, int h, int oldw, int oldh) {

        super.onSizeChanged(w, h, oldw, oldh);
        if(w>0 && h>0) {
            if(imageBitmap!=null&& imageBitmap.getHeight()>0 && imageBitmap.getWidth()>0) {
                //create seeds layer
                seedsBitmap = Bitmap.createBitmap(imageBitmap.getWidth(), imageBitmap.getHeight(), Bitmap.Config.ARGB_8888);
                //create segmentation layer
                segmentationBitmap = Bitmap.createBitmap(imageBitmap.getWidth(), imageBitmap.getHeight(), Bitmap.Config.ARGB_8888);
                //to draw on seeds layer
                seedsCanvas = new Canvas(seedsBitmap);
                organizeScreen(w, h-dpToPx(100), imageBitmap.getWidth(), imageBitmap.getHeight());
                //image rectangle
                imageRect = new Rect(0, 0, imageBitmap.getWidth(), imageBitmap.getHeight());
                //part of the image displayed in zoom area
                partImageArea = new Rect(x0Zoom, y0Zoom, x0Zoom + zoomArea.width(), y0Zoom + zoomArea.height());
            }
        }
    }

    @Override
    protected void onDraw(Canvas canvas) {
        //draw image
        canvasPaint.setAlpha(255);
        canvas.drawBitmap(imageBitmap,imageRect,miniatureArea,canvasPaint);
        canvas.drawBitmap(imageBitmap,partImageArea,zoomArea,canvasPaint);
        //draw segmentation
        canvasPaint.setAlpha(128);
        canvas.drawBitmap(segmentationBitmap,imageRect,miniatureArea,canvasPaint);
        canvas.drawBitmap(segmentationBitmap,partImageArea,zoomArea,canvasPaint);
        //draw seeds
        canvasPaint.setAlpha(255);
        canvas.drawBitmap(seedsBitmap,partImageArea,zoomArea,canvasPaint);
        canvas.drawBitmap(seedsBitmap,imageRect,miniatureArea,canvasPaint);
    }

    /**
     * Set segmentation layer
     * @param image
     */
    public void setSegmentationLayer(Bitmap image){
        segmentationBitmap= image;
        invalidate();

    }


    @Override
    public boolean onTouchEvent(MotionEvent event) {
        //retrieve the X and Y positions of the user touch
        float touchX = event.getX();
        float touchY = event.getY();

        if(zoomArea.contains((int)touchX,(int)touchY)) {
            //when the user touches the View, move to that position to start drawing
            //when they move their finger on the View, draw the path along with their touch
            //when they lift their finger up off the View, draw the Path and reset it for the next
            //drawing operation
            switch (event.getAction()) {
                case MotionEvent.ACTION_DOWN:
                    seedsPath.moveTo(touchX-zoomArea.left+x0Zoom,touchY-zoomArea.top+y0Zoom);
                    state++;
                    break;
                case MotionEvent.ACTION_MOVE:
                    if(state>0) {
                        seedsPath.lineTo(touchX-zoomArea.left+x0Zoom,touchY-zoomArea.top+y0Zoom);
                    }
                    break;
                case MotionEvent.ACTION_UP:
                    if(state>0) {
                        seedsCanvas.drawPath(seedsPath,drawPaint);
                        seedsPath.reset();
                        state=0;
                    }

                    break;
                default:
                    return false;
            }
            //will cause the "onDraw" method to execute
            invalidate();
        }
        return true;
    }

    /**
     * Change brush color
     * @param newColor
     */
    public void setColor(String newColor){
        //set color
        invalidate();
        //parse and set color for drawing
        paintColor = Color.parseColor(newColor);
        drawPaint.setColor(paintColor);
    }

    /**
     * Change brush size
     * @param newSize
     */
    public void setBrushSize(float newSize){
        float pixelAmount = TypedValue.applyDimension(TypedValue.COMPLEX_UNIT_DIP,
                newSize, getResources().getDisplayMetrics());
        brushSize=pixelAmount;
        drawPaint.setStrokeWidth(brushSize);
    }

    /**
     * Change last brush size
     * @param lastSize
     */
    public void setLastBrushSize(float lastSize){
        lastBrushSize=lastSize;
    }

    /**
     * Last brush size getter
     * @return
     */
    public float getLastBrushSize(){
        return lastBrushSize;
    }

    /**
     * Activate or not eraser
     * @param isErase
     */
    public void setErase(boolean isErase){
        erase=isErase;
        if(erase){
            //alter the Paint object to erase
            drawPaint.setXfermode(new PorterDuffXfermode(PorterDuff.Mode.CLEAR));
        }else{
            //switch back to drawing
            drawPaint.setXfermode(null);
        }
    }

    public boolean getEraser(){
        return erase;
    }

    public Bitmap getSeeds(){
        return seedsBitmap;
    }

    public Bitmap getSegmentation(){

        return segmentationBitmap;

    }

    public void setSeeds(Bitmap newSeeds){
        this.seedsBitmap=newSeeds;
        invalidate();
    }

    public Bitmap getImage() { return imageBitmap;}

    public void moveDownZoom(){
        y0Zoom=Math.min(y0Zoom+moveStep,imageBitmap.getHeight()-zoomArea.height());
        partImageArea=new Rect(x0Zoom,y0Zoom,x0Zoom+zoomArea.width(),y0Zoom+zoomArea.height());
        //will cause the "onDraw" method to execute
        invalidate();
    }


    public void moveUpZoom(){
        y0Zoom=Math.max((y0Zoom-moveStep),0);
        partImageArea=new Rect(x0Zoom,y0Zoom,x0Zoom+zoomArea.width(),y0Zoom+zoomArea.height());
        //will cause the "onDraw" method to execute
        invalidate();
    }

    public void moveRightZoom(){
        x0Zoom=Math.min(x0Zoom+moveStep,imageBitmap.getWidth()-zoomArea.width());
        partImageArea=new Rect(x0Zoom,y0Zoom,x0Zoom+zoomArea.width(),y0Zoom+zoomArea.height());
        //will cause the "onDraw" method to execute
        invalidate();
    }

    public void moveLeftZoom(){
        x0Zoom=Math.max((x0Zoom-moveStep),0);
        partImageArea=new Rect(x0Zoom,y0Zoom,x0Zoom+zoomArea.width(),y0Zoom+zoomArea.height());
        //will cause the "onDraw" method to execute
        invalidate();
    }

}
