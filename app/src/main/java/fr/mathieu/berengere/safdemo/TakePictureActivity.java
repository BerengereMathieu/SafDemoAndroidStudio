package fr.mathieu.berengere.safdemo;

import android.app.Activity;
import android.os.Bundle;

public class TakePictureActivity extends Activity {
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_take_picture);
        if (null == savedInstanceState) {

            //Intent intent = getIntent();
            getFragmentManager().beginTransaction()
                    .replace(R.id.container, CameraActivity.newInstance(this,"",""))
                    .commit();

        }
    }


}