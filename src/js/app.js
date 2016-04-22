
function fetchWeather(latitude, longitude) {
  var req = new XMLHttpRequest();
  req.open('GET', 'http://api.openweathermap.org/data/2.5/weather?' +
    'lat=' + latitude + '&lon=' + longitude + '&cnt=1&appid=' + myAPIKey, true);
  req.onload = function () {
    if (req.readyState === 4) {
      if (req.status === 200) {
        console.log(req.responseText);
        var response = JSON.parse(req.responseText);
        var temperature = Math.round(response.main.temp - 273.15);
        var icon = iconFromWeatherId(response.weather[0].id);
        var city = response.name;
        console.log(temperature);
        console.log(icon);
        console.log(city);
        Pebble.sendAppMessage({
          'WEATHER_ICON_KEY': icon,
          'WEATHER_TEMPERATURE_KEY': temperature + '\xB0C',
          'WEATHER_CITY_KEY': city
        });
      } else {
        console.log('Error');
      }
    }
  };
  req.send(null);
}


Pebble.addEventListener('ready', function (e) {
  console.log('connect!', JSON.stringify(e));
});

Pebble.addEventListener('appmessage', function (e) {
  console.log('appmessage', JSON.stringify(e));
});

Pebble.addEventListener('webviewclosed', function (e) {
  console.log('webview closed', JSON.stringify(e));
});
