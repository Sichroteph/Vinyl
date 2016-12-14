var xhrRequest = function (url, type, callback) {
  var xhr = new XMLHttpRequest();
  xhr.onload = function () {
    callback(this.responseText);
  };
  xhr.open(type, url);
  xhr.send();
};


function getForecast() {
  console.log("getForecast");
  var temperature;

  var coordinates = localStorage.getItem(153);
  var units = localStorage.getItem(152);


  var units_s;
  if(units==1){
    units_s="us";
  }
  else{
    units_s="ca";
  }


  var api_key;


  api_key= ' ';
  

  var url = 'https://api.forecast.io/forecast/'+api_key+'/'+coordinates+'?units='+units_s; 
  url=url.replace(/"/g,"");

  //console.log("getWeather url "+url);

  xhrRequest(url, 'GET', 
             function(responseText) {
               console.log("responseText");
               var json = JSON.parse(responseText);
               if(units==1)
                 temperature = Math.round(json.currently.temperature)+"°F";
               else
                 temperature = Math.round(json.currently.temperature)+"°C";
               console.log("temperature "+temperature);

               var dict = {};
               dict.KEY_TEMPERATURE=temperature;

               // Send to watchapp
               Pebble.sendAppMessage(dict, function() {
                 console.log("sendAppMessage");
                 console.log('Send successful: ' + JSON.stringify(dict));
               }, function() {
                 console.log('Send failed!');
               }
                                    );


             });





}





function locationSuccess(pos) {
  console.log("locationSuccess");
  var lat= pos.coords.latitude;
  var lng= pos.coords.longitude;
  var coordinates=lat+','+lng;

  localStorage.setItem(153, JSON.stringify(coordinates));
  getForecast();

}

function locationError(err) {
  console.log("locationError");
  // console.log("Error requesting location!");
}

function getWeather() {
  navigator.geolocation.getCurrentPosition(
    locationSuccess,
    locationError,
    {timeout: 15000, maximumAge: 60000}
  );
}



// Listen for when the watchface is opened
Pebble.addEventListener('ready', 
                        function(e) {
                          console.log("PebbleKit JS ready!");
                         // getWeather();
                        }
                       );

// Listen for when an AppMessage is received
Pebble.addEventListener('appmessage',
                        function(e) {
                          console.log("AppMessage received!");
                          getWeather();  
                        }                     
                       );



Pebble.addEventListener('showConfiguration', function() {
  //var url = 'http://sichroteph.github.io/Ruler-Weather/';
  var url = 'http://sichroteph.github.io/Analog-Cardinal/';

  //  console.log('Showing configuration page: ' + url);
  Pebble.openURL(url);
});





Pebble.addEventListener('webviewclosed', function(e) {
  var configData = JSON.parse(decodeURIComponent(e.response));
  // console.log('Configuration page returned: ' + JSON.stringify(configData));

  var vibration = configData['vibration'];
  var mn_marks = configData['mn_marks'];
  var five_marks = configData['five_marks'];


  var color_bg = configData['color_bg'];
  var color_hands = configData['color_hands'];
  var color_bubles = configData['color_bubles'];
  var color_text = configData['color_text'];

  var radio_units = configData.radio_units;

  var dict = {};

  dict['KEY_VIBRATION'] = configData['vibration'] ? 1 : 0;  // Send a boolean as an integer
  dict['KEY_MN_MARKS'] = configData['mn_marks'] ? 1 : 0;;
  dict['KEY_FIVE_MARKS'] = configData['five_marks'] ? 1 : 0;;

  dict['KEY_COLOR_BG_R'] = parseInt(color_bg.substring(2, 4), 16);
  dict['KEY_COLOR_BG_G'] = parseInt(color_bg.substring(4, 6), 16);
  dict['KEY_COLOR_BG_B'] = parseInt(color_bg.substring(6, 8), 16);

  dict['KEY_COLOR_HANDS_R'] = parseInt(color_hands.substring(2, 4), 16);
  dict['KEY_COLOR_HANDS_G'] = parseInt(color_hands.substring(4, 6), 16);
  dict['KEY_COLOR_HANDS_B'] = parseInt(color_hands.substring(6, 8), 16);


  dict['KEY_COLOR_BUBLES_R'] = parseInt(color_bubles.substring(2, 4), 16);
  dict['KEY_COLOR_BUBLES_G'] = parseInt(color_bubles.substring(4, 6), 16);
  dict['KEY_COLOR_BUBLES_B'] = parseInt(color_bubles.substring(6, 8), 16);

  dict['KEY_COLOR_TEXT_R'] = parseInt(color_text.substring(2, 4), 16);
  dict['KEY_COLOR_TEXT_G'] = parseInt(color_text.substring(4, 6), 16);
  dict['KEY_COLOR_TEXT_B'] = parseInt(color_text.substring(6, 8), 16);

  localStorage.setItem(152, radio_units ? 1 : 0);

  // Send to watchapp
  Pebble.sendAppMessage(dict, function() {
    // console.log('Send successful: ' + JSON.stringify(dict));
  }, function() {
    // console.log('Send failed!');
  }
                       );

  // Persist write a key with associated value



  // refresh weather
  getWeather();


});
