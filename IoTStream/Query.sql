SELECT
    name, System.TimeStamp As OutTime, AVG(CAST(Humidity AS FLOAT)) AS avgHumidity, AVG(CAST(TemperatureH AS FLOAT)) AS avgTemperatureH, AVG(CAST(Pressure AS FLOAT)) AS avgPressure, AVG(CAST(TemperatureP AS FLOAT)) AS avgTemperatureP, AVG(CAST(Light AS FLOAT)) AS avgLight, AVG(CAST(Battery AS FLOAT)) AS avgBatteryAS
INTO
    WeatherStationOutput
FROM
    WeatherStationInput
GROUP BY name, TumblingWindow(second, 20)