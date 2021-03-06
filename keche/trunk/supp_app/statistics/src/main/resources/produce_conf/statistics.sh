#!/bin/sh

LBS_HOME="/home/zjhl/supp_app/statistics"
LOGBACK="-Dlogback.configurationFile=$LBS_HOME/conf/logback.xml"
JVM_LOG_DIR="/home/zjhl/supp_app/statistics/logs/"

PIDFILE="$LBS_HOME/PID"
SERVICE_CLASS="com.ctfo.storage.statistics.core.StatisticsMain"

if [ "$JAVA_HOME" = "" ]; then
	JAVA_HOME="/usr/java/jdk1.6.0_45"
fi

DATE_STRING="$(date +'%Y-%m-%d-%H-%M')"
JAVACMD="$JAVA_HOME/bin/java"
export LC_ALL=zh_CN
oldCP=$CLASSPATH
unset CLASSPATH

for i in ${LBS_HOME}/lib/*.jar ; do
  if [ "$CLASSPATH" != "" ]; then
    CLASSPATH=${CLASSPATH}:$i
  else
    CLASSPATH=$i
  fi
done

CLASS_HOME=""
if [ "$CLASS_HOME" != "" ]; then
  for i in ${CLASS_HOME}/* ; do
    if [ "$CLASSPATH" != "" ]; then
      CLASSPATH=${CLASSPATH}:$i
    else
      CLASSPATH=$i
    fi
  done
fi

if [ "$oldCP" != "" ]; then
    CLASSPATH=${CLASSPATH}:${oldCP}
fi

JVM_LOG="${JVM_LOG_DIR}jvm-${DATE_STRING}.log"

INIT_PARAM="-Xmx1512m -Xms1512m -Xmn784m -XX:PermSize=128m -XX:MaxPermSize=256m -XX:+UseConcMarkSweepGC -XX:+UseCMSCompactAtFullCollection -XX:CMSMaxAbortablePrecleanTime=50 -XX:+CMSClassUnloadingEnabled -XX:+PrintGC -XX:+PrintGCDateStamps -Xloggc:$JVM_LOG"

case "$1" in
	start)
		echo ${JAVACMD} ${LOGBACK}  ${INIT_PARAM} -cp $CLASSPATH ${SERVICE_CLASS} -start
		if [ -f $PIDFILE ]
                then
                        echo "$PIDFILE exists, Statistics is already running or crashed."
                else
                        if [ `whoami` = "zjhl" ]; then
                        	if [ -d "$JVM_LOG_DIR" ]; then
			        	echo "JVM_LOG_DIR=${JVM_LOG_DIR}"    
				else
				    	mkdir $JVM_LOG_DIR
				fi
				if [ -f "$JVM_LOG" ]; then
				   	echo "JVM_LOG=${JVM_LOG}"
				else
				   	touch $JVM_LOG
				fi
    				echo "Starting Statistics server..."
				${JAVACMD} $LOGBACK $INIT_PARAM -cp $CLASSPATH ${SERVICE_CLASS} -d  conf -start &
                        else 
                            echo "Current User Init Error!"
                        fi
                fi
                if [ "$?"="0" ]
                then
                        echo "Statistics is Loading..."
                fi
                ;;
	stop)
                if [ ! -f $PIDFILE ]
                then
                        echo "$PIDFILE exists, Statistics is not running."
                else
                        PID=$(cat $PIDFILE)
			echo "Statistics Stopping..."
                        kill -9 $PID & rm -rf $PIDFILE &  echo "kill Statistics ok!"
			sleep 2
                        while [ -x $PIDFILE ]
                        do
                                echo "Waiting for Statistics to shutdown..."
                                sleep 1
                        done
                        echo "Statistics stopped!"
                fi
                ;;
       restart)
                ${0} stop 
                sleep 3
 		        ${0} start
                ;;	

	*)
		echo "Usage: $0 {start}"
esac

exit 0
