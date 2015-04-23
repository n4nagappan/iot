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

function setPosition(){
    $.get('/location',function( data ) {
        console.log("Fetched location : " + JSON.stringify(data));
        var myLatlng = new google.maps.LatLng(data.lat, data.long);
        var marker = new google.maps.Marker({   
          position: myLatlng,
          map: map,
          title: 'I am here'
        });
        map.setCenter(myLatlng);
    });
}