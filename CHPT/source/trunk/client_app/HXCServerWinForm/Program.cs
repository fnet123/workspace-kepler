﻿using System;
using System.Collections.Generic;
using System.Linq;
using System.Windows.Forms;
using System.IO;
using Utility.Log;
using HXCServerWinForm.LoginForms;
using System.Threading;
using ServiceStationClient.ComponentUI;
using Utility.Common;
using HXC_FuncUtility;

namespace HXCServerWinForm
{
    static class Program
    {
        /// <summary>
        /// 应用程序的主入口点。
        /// </summary>
        [STAThread]
        static void Main()
        {
            if (Common.IsCurrentProcessExist())
            {
                MessageBoxEx.ShowWarning("对不起,本地已经有系统正在运行!\n");
                System.Environment.Exit(0);
            }

            //处理未捕获的异常   
            Application.SetUnhandledExceptionMode(UnhandledExceptionMode.CatchException);
            //处理UI线程异常   
            Application.ThreadException += new System.Threading.ThreadExceptionEventHandler(Application_ThreadException);
            //处理非UI线程异常   
            AppDomain.CurrentDomain.UnhandledException += new UnhandledExceptionEventHandler(CurrentDomain_UnhandledException);

            Application.EnableVisualStyles();
            Application.SetCompatibleTextRenderingDefault(false);
            //LoginForm Login = new LoginForm();
            FormLogin Login = new FormLogin();
            Login.ShowDialog();//显示登陆窗体           
            if (Login.DialogResult == DialogResult.OK)
            {
                GlobalStaticObj.AppMainForm = new HXCServerMainForm();
                Application.Run(GlobalStaticObj.AppMainForm);//判断登陆成功时主进程显示主窗口  
            }
        }

        /// <summary>
        ///这就是我们要在发生未处理异常时处理的方法，我这是写出错详细信息到文本，如出错后弹出一个漂亮的出错提示窗体，给大家做个参考
        ///做法很多，可以是把出错详细信息记录到文本、数据库，发送出错邮件到作者信箱或出错后重新初始化等等
        ///这就是仁者见仁智者见智，大家自己做了。
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        static void Application_ThreadException(object sender, System.Threading.ThreadExceptionEventArgs e)
        {

            string str = "";
            string strDateInfo = "出现应用程序未处理的异常：" + DateTime.Now.ToString() + "\r\n";
            Exception error = e.Exception as Exception;
            if (error != null)
            {
                str = string.Format(strDateInfo + "异常类型：{0}\r\n异常消息：{1}\r\n异常信息：{2}\r\n",
                     error.GetType().Name, error.Message, error.StackTrace);
            }
            else
            {
                str = string.Format("应用程序线程错误:{0}", e);
            }
            GlobalStaticObj_Server.GlobalLogService.WriteLog("system", str);
            MessageBoxEx.ShowWarning("系统发生未知错误，请及时联系软件提供商！");
        }

        static void CurrentDomain_UnhandledException(object sender, UnhandledExceptionEventArgs e)
        {
            string str = "";
            Exception error = e.ExceptionObject as Exception;
            string strDateInfo = "出现应用程序未处理的异常：" + DateTime.Now.ToString() + "\r\n";
            if (error != null)
            {
                str = string.Format(strDateInfo + "Application UnhandledException:{0};\n\r堆栈信息:{1}", error.Message, error.StackTrace);
            }
            else
            {
                str = string.Format("Application UnhandledError:{0}", e);
            }

            GlobalStaticObj_Server.GlobalLogService.WriteLog("system", str);
            MessageBoxEx.ShowWarning("系统发生未知错误，请及时联系软件提供商！");
        }
    }
}
