using System.Net;
using Microsoft.Azure.Devices.Client;
using System.Text;
using Newtonsoft.Json;

public static async Task<HttpResponseMessage> Run(System.Net.Http.HttpRequestMessage req, TraceWriter log)
{
    // Get request body
    dynamic data = await req.Content.ReadAsAsync<object>();
    string name = data?.name;
    log.Info("data " + data);

    string s_connectionString = "HostName=IoTHubMidgard.azure-devices.net;DeviceId=WeatherStation;SharedAccessKey=NvDtfOSUsaAPF3RovrL2ZHsceSheTzSb4UJPtRrVabI=";
    DeviceClient s_deviceClient = DeviceClient.CreateFromConnectionString(s_connectionString, Microsoft.Azure.Devices.Client.TransportType.Mqtt);

    // Create JSON message
    var telemetryDataPoint = new
    {
        Humidity = data?.Humidity,
        TemperatureH = data?.TemperatureH,
        Pressure = data?.Pressure,
        TemperatureP = data?.TemperatureP,
        Light = data?.Light,
        Battery = data?.Battery,
    };
    var messageString = JsonConvert.SerializeObject(telemetryDataPoint);
    var message = new Message(Encoding.ASCII.GetBytes(messageString));
    await s_deviceClient.SendEventAsync(message);

    return req.CreateResponse(HttpStatusCode.OK, "Success");
}
