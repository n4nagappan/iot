var map;

function initialize() {
  var myLatlng = new google.maps.LatLng(1.3147308,103.8470128);
  var mapOptions = {
    zoom: 16,
    center: myLatlng
  }
  map = new google.maps.Map(document.getElementById('map-canvas'), mapOptions);
}

google.maps.event.addDomListener(window, 'load', initialize);

$(document).ready(function(){
    setInterval(setPosition, 5000);
});

function ConvertDMSToDD(degrees, minutes) {
    var dd = degrees + ( minutes*60)/3600;
    return dd;
}

function convert(x){
    var direction = x[ x.length - 1];
    x = x.substring(0, x.length - 1);
    var parts = x.split('.');
    var prefix = parts[0];
    var degrees = parseInt(prefix.substring(0, prefix.length - 2));
    var m = parseFloat(prefix.substring(prefix.length - 2) +"."+ parts[1]);
    var val = ConvertDMSToDD(degrees, m );
    return val;
}

var marker;
function setPosition(){
    $.get('/location',function( data ) {
        console.log("Fetched location : " + JSON.stringify(data));

        var myLatlng = new google.maps.LatLng(data.lat, data.long);
        marker = new google.maps.Marker({   
          position: myLatlng,
          map: map,
          title: "I am here"
        });

        map.setCenter(myLatlng);
    });
}
