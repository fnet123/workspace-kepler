package com.caits.analysisserver.quartz.jobs.impl;

import java.sql.Connection;
import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.util.Date;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import com.caits.analysisserver.database.OracleConnectionPool;
import com.caits.analysisserver.database.SQLPool;
import com.caits.analysisserver.utils.CDate;

/**
 * 
 * <p>
 * -----------------------------------------------------------------------------
 * <br>
 * 工程名 ： StatisticAnalysisServer <br>
 * 功能： <br>
 * 描述： <br>
 * 授权 : (C) Copyright (c) 2011 <br>
 * 公司 : 北京中交兴路信息科技有限公司 <br>
 * -----------------------------------------------------------------------------
 * <br>
 * 修改历史 <br>
 * <table width="432" border="1">
 * <tr>
 * <td>版本</td>
 * <td>时间</td>
 * <td>作者</td>
 * <td>改变</td>
 * </tr>
 * <tr>
 * <td>1.0</td>
 * <td>2013-01-16</td>
 * <td>yujch</td>
 * <td>创建</td>
 * </tr>
 * </table>
 * <br>
 * <font color="#FF0000>注意: 本内容仅限于[北京中交兴路信息科技有限公司]内部使用，禁止转发</font> <br>
 * 
 * @version 1.0
 * 
 * @author yujch
 * @since JDK1.6
 */
public class VehicleOverspeedAnalyseDaystatJobdetail {

	private static final Logger logger = LoggerFactory
			.getLogger(VehicleOverspeedAnalyseDaystatJobdetail.class);

	// ------获得xml拼接的Sql语句
	private String delvehicleOverspeedRateDaystatSql;// 删除车辆超速比率日统计结果
	private String addvehicleOverspeedRateDaystatSql;// 统计车辆超速比率情况
	private String addvehicleOverspeedDaystatSql;// 统计车辆日超速情况

//	private int count = 0;// 计数器

	private long statDate;
	private long beginTime;
	private long endTime;

	/**
	 * 初始化统计周期：传入日期
	 * 
	 * @param statDate
	 *            当日12点日期时间
	 */
	public VehicleOverspeedAnalyseDaystatJobdetail(Date currDay) {
		this.statDate = currDay.getTime();
		this.beginTime = statDate - 1000 * 60 * 60 * 12;
		this.endTime = statDate + 1000 * 60 * 60 * 12;

		this.initAnalyser();
	}

	// 初始化方法
	public void initAnalyser() {
		// 删除企业日车辆运营情况
		delvehicleOverspeedRateDaystatSql = SQLPool.getinstance().getSql(
				"sql_delVehicleOverspeedRateDaystat");
		// 统计企业日车辆运营情况
		addvehicleOverspeedRateDaystatSql = SQLPool.getinstance().getSql(
				"sql_vehicleOverspeedRateDaystat");
		// 统计企业日车辆运营情况
		addvehicleOverspeedDaystatSql = SQLPool.getinstance().getSql(
				"sql_vehicleOverspeedDaystat");

	}

	/**
	 * 生成车辆日运营属性
	 * 
	 * @param
	 * @return int 0:执行失败, 1执行成功
	 */
	public int executeStatRecorder() {
		PreparedStatement dbPstmt0 = null;
		PreparedStatement dbPstmt1 = null;
		PreparedStatement dbPstmt2 = null;
		Connection dbConnection = null;

		// 结果集对象
		ResultSet dbResultSet = null;

		// 成功标志位 0:执行失败, >=1执行成功,成功解析个数
		int flag = 0;
		try {
			// 获得Connection对象
			dbConnection = OracleConnectionPool.getConnection();
			if (dbConnection != null) {
				
				// 删除临时表中统计数据
				 dbPstmt0 = dbConnection.prepareStatement(delvehicleOverspeedRateDaystatSql);
				 dbPstmt0.setLong(1, beginTime);
				 dbPstmt0.setLong(2, endTime);
				 dbPstmt0.setLong(3, beginTime);
				 dbPstmt0.setLong(4, endTime);
				 dbPstmt0.executeUpdate();

				// 生成车辆日超速比率中间表数据
				dbPstmt1 = dbConnection
						.prepareStatement(addvehicleOverspeedRateDaystatSql);
				dbPstmt1.setLong(1, beginTime);
				dbPstmt1.setLong(2, endTime);
				dbPstmt1.executeUpdate();

				// 生成车辆超速统计结果表数据
				dbPstmt2 = dbConnection
						.prepareStatement(addvehicleOverspeedDaystatSql);
				dbPstmt2.setLong(1, statDate);
				dbPstmt2.setLong(2, beginTime);
				dbPstmt2.setLong(3, endTime);
				
				dbPstmt2.executeUpdate();

				Date dt = new Date();
				dt.setTime(this.statDate);

				logger.debug(CDate.dateToStr(dt) + "企业车辆日超速情况统计信息生成成功！");
				flag = 1;
			} else {
				logger.debug("获取数据库链接失败");
			}
		} catch (Exception e) {
			logger.error("生成企业车辆日超速情况信息出错：", e);
			flag = 0;
		} finally {
			try {
				if (dbResultSet != null) {
					dbResultSet.close();
				}
				if (dbPstmt0 != null) {
					dbPstmt0.close();
				}
				if (dbPstmt1 != null) {
					dbPstmt1.close();
				}
				if (dbPstmt2 != null) {
					dbPstmt2.close();
				}
				if (dbConnection != null) {
					dbConnection.close();
				}
			} catch (SQLException e) {
				logger.error("连接放回连接池出错.", e);
			}
		}
		return flag;
	}

	/**
	 * 将空值转换为空字符串
	 * 
	 * @param str
	 *            字符串
	 * @return String 返回处理后的字符串
	 */
	public static String nullToStr(String str) {
		return str == null || str.equals("null") ? "" : str.trim();
	}

}
