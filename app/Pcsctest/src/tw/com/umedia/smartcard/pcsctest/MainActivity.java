package tw.com.umedia.smartcard.pcsctest;

import tw.com.umedia.smartcard.service.PCSCService;
import android.app.Activity;
import android.os.Bundle;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.TextView;
import android.util.Log;

public class MainActivity extends Activity implements OnClickListener {
	private static final String TAG="PcscTest";

	public TextView mTextView;
	public Button mButton;

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);
		mTextView = (TextView) findViewById(R.id.logwindow);
		mButton = (Button) findViewById(R.id.button_start);
		mButton.setOnClickListener(this);
	}

	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		// Inflate the menu; this adds items to the action bar if it is present.
		getMenuInflater().inflate(R.menu.main, menu);
		return true;
	}

	@Override
	public boolean onOptionsItemSelected(MenuItem item) {
		// Handle action bar item clicks here. The action bar will
		// automatically handle clicks on the Home/Up button, so long
		// as you specify a parent activity in AndroidManifest.xml.
		int id = item.getItemId();
		if (id == R.id.action_settings) {
			return true;
		}
		return super.onOptionsItemSelected(item);
	}

	@Override
	public void onClick(View v) {
		// TODO Auto-generated method stub
		ProcessPcsc();
	}
	
	public void ProcessPcsc() {
		long ret;
		String text;
		PCSCService mPcsc;
		mPcsc = new PCSCService();
		
		mTextView.setText("");

		ret = mPcsc.EstablishContext(PCSCService.SCARD_SCOPE_SYSTEM);
		if(ret != PCSCService.SCARD_S_SUCCESS) {
			text = mTextView.getText() + "Testing SCardEstablishContext Error =" + Long.toHexString(ret) + "\n";
			mTextView.setText(text);
			return;
		}
		text = mTextView.getText() + "Testing SCardEstablishContext OK!\n";
		mTextView.setText(text);

		ret = mPcsc.IsValidContext();
		if(ret != PCSCService.SCARD_S_SUCCESS) {
			text = mTextView.getText() + "Testing SCardIsValidContext Error =" + Long.toHexString(ret) + "\n";
			mTextView.setText(text);
			return;
		}
		text = mTextView.getText() + "Testing SCardIsValidContext OK!\n";
		mTextView.setText(text);

		ret = mPcsc.ListReaderGroups();
		if( (ret&0x80100000) > 0 ) {
			text = mTextView.getText() + "Testing SCardListReaderGroups Error =" + Long.toHexString(ret) + "\n";
			mTextView.setText(text);
			return;
		}
		text = mTextView.getText() + "Testing SCardListReaderGroups ret = " + ret + "\n";
		mTextView.setText(text);
		
		String[] readers = mPcsc.ListReaders();
                text = mTextView.getText() + "Testing SCardListReaders!\n";
		if(readers!=null && readers.length > 0) {
			for(int i=0; i < readers.length; i++) {
				if(readers[i]!=null) {
					text = text + "Reader " + i + " : " + readers[i] + "\n";
				}
				Log.d(TAG,"Reader " + (i+1) + " : " + readers[i]);
                        }
			mTextView.setText(text);
                }

		ret = mPcsc.GetStatusChange(3);
		if( (ret&0x80100000) > 0 ) {
			text = mTextView.getText() + "Testing SCardGetStatusChange Error =" + Long.toHexString(ret) + "\n";
			mTextView.setText(text);
			return;
		}
		text = mTextView.getText() + "Testing SCardGetStatusChange ret = " + Long.toHexString(ret) + "\n";
		mTextView.setText(text);
		if((ret&PCSCService.SCARD_STATE_UNKNOWN)> 0) {
			return;
		}

		ret = mPcsc.Connect(3, PCSCService.SCARD_SHARE_SHARED);
		if(ret != PCSCService.SCARD_S_SUCCESS) {
			text = mTextView.getText() + "Testing SCardConnect Error =" + Long.toHexString(ret) + "\n";
			mTextView.setText(text);
			return;
		}
		text = mTextView.getText() + "Testing SCardConnect OK!\n";
		mTextView.setText(text);

		byte[] xdata = new byte[] { (byte)0x00, (byte)0xa4, (byte)0x00, (byte)0x00, (byte)0x02, (byte)0x3f, (byte)0x00 };
		byte[] rdata = mPcsc.Transmit(xdata);
		if(rdata == null || rdata.length == 0) {
			text = mTextView.getText() + "Testing SCardTransmit Error!\n";
			mTextView.setText(text);
			//return;
		} else {
			text = mTextView.getText() + "Testing SCardTransmit OK!\n rdata = ";
                	for(int i=0; i < rdata.length; i++) {
                    		text += Integer.toHexString(rdata[i]&0xff) + " ";
                	}
			text += "\n";
			mTextView.setText(text);
		}

		xdata = null;
		rdata = null;
		xdata = new byte[] { 0x02 };
		rdata = mPcsc.Control(1, xdata);
		if(rdata == null || rdata.length == 0) {
			text = mTextView.getText() + "Testing SCardControl Error!\n";
			mTextView.setText(text);
			//return;
		} else {
			text = mTextView.getText() + "Testing SCardControl OK!\n rdata = ";
        	        for(int i=0; i < rdata.length; i++) {
        	            text += Integer.toHexString(rdata[i]&0xff) + " ";
        	        }
			mTextView.setText(text);
		}
	
		byte[] atrdata = mPcsc.GetAttrib(PCSCService.SCARD_ATTR_ATR_STRING);
		if(atrdata == null || atrdata.length == 0) {
			text = mTextView.getText() + "Testing SCardGetAttrib Error!\n";
			mTextView.setText(text);
			//return;
		} else {
			text = mTextView.getText() + "Testing SCardGetAttrib OK!\n atrdata = ";
        	        for(int i=0; i < atrdata.length; i++) {
        	            text += Integer.toHexString(atrdata[i]&0xff) + " ";
        	        }
			mTextView.setText(text);
		}

	}
	
	
	
}
