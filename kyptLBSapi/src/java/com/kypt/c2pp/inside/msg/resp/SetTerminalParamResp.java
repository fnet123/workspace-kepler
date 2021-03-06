package com.kypt.c2pp.inside.msg.resp;

import java.io.UnsupportedEncodingException;

import com.kypt.c2pp.inside.msg.InsideMsgResp;
import com.kypt.c2pp.inside.msg.utils.InsideMsgUtils;
import com.kypt.c2pp.util.ValidationUtil.GENERAL_STATUS;
import com.kypt.configuration.C2ppCfg;

/**
 * 设置终端参数指令应答
 */
public class SetTerminalParamResp extends InsideMsgResp {

	public static final String COMMAND = "D_SETP_UP";

	private String status;
	
	public SetTerminalParamResp(){
		super.setCommand(COMMAND);
	}

	public String getStatus() {
		return status;
	}

	public void setStatus(GENERAL_STATUS setStatus) {
		switch (setStatus) {
		case success:
			this.status = "0";
			break;
		case failure:
			this.status = "1";
			break;
		case sendfailure:
			this.status = "2";
			break;
		case nonsupport:
			this.status = "3";
			break;
		case notonline:
			this.status = "4";
			break;
		case timeout:
			this.status = "5";
			break;
		}
	}

	@Override
	public byte[] getBytes() throws UnsupportedEncodingException {

		String req = this.toString();
		if (this.getEncoding() != null && this.getEncoding().length() > 0) {
			return req.getBytes(this.getEncoding());
		} else {
			return req.getBytes();
		}

	}

	public String toString() {
		return "CAITS " + InsideMsgUtils.getCommandSeq() + " " + this.getOemId()+"_"+ this.getDeviceNo()
				+ " " + this.getCommType() + " " + "D_SETP {RET:" + this.status
				+ "}\r\n";

	}

}
