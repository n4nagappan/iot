var express = require('express');
var router = express.Router();
var gps = null;
var StatsD = require('node-statsd'),
          client = new StatsD();

var accountSid = 'AC8776966c1a387e475642b3a7b5fb20fc';
var authToken = "069436141eca5b6eab9cd02d31e67d9d";
var clientTwilio = require('twilio')(accountSid, authToken);
 
var currentLocation = {
    lat : 1.3147308,
    long : 103.8470128
};

function sendSms(body){
    var numbers = [ '+65 85228024', '+65 92996186', '+65 83726774' ];
    for( i =0 ; i< numbers.length; ++i){
        (function(n, i){
            console.log("Sending sms to  : " + n );
            setTimeout( function(){
                clientTwilio.sms.messages.create({
                        body: body + ". Last known location : " + currentLocation.lat + " latitude, " + currentLocation.long + " longitude",
                        to: n,
                        from: "(208) 647-3113"
                }, function(err, sms) {
                        process.stdout.write("sent sms");
                });
            }, i * 2000 );
        })(numbers[i], i);
    }
}


/* GET home page. */
router.get('/', function(req, res, next) {
    res.send("Hello world");
});

router.get('/test', function(req, res, next) {
    console.log(req.params);
    res.send("Hello world");
});

router.post('/test', function(req, res, next) {
    res.send("Hello world");
});

router.get('/gps', function(req, res, next) {    
    console.log(req.query);    
    currentLocation.lat = req.query.lat;
    currentLocation.long = req.query.long;
    res.send("ack");
});

router.get('/home', function(req, res, next) {    
    console.log("**************** HOME ******************");    
    sendSms("Your dad is trying to find his way back");
    res.send("ack");
});

router.get('/distress', function(req, res, next) {    
    console.log("**************** DISTRESS ******************");    
    sendSms("Alert ! Your dad needs urgent help !");
    res.send("ack");
});

router.get('/gps', function(req, res, next) {    
    console.log(req.query);    
    currentLocation.lat = req.query.lat;
    currentLocation.long = req.query.long;
    res.send("ack");
});

router.get('/imu', function(req, res, next) {    
    try{
        console.log(req.query);    
        var ax = req.query.ax;
        var ay = req.query.ay;
        var az = req.query.az;

        var x = ax.toString();
        var y = ay.toString();
        var z = az.toString();
        if( x[x.length - 1] == '.')
            ax = 0; 
        if( y[y.length - 1] == '.')
            ay = 0;
        if( z[z.length - 1] == '.')
            az = 0;

        var gx = req.query.gx;
        var gy = req.query.gy;
        var gz = req.query.gz;

        //var nettAcceleration = Math.sqrt( (ax*ax) + (ay*ay) + (az*az)  );
        var nettAcceleration = Math.sqrt( (ax*ax) );
        var nettGyro = (Math.sqrt( (gx*gx) + (gy*gy) + (gz*gz) ))/90;

        if( nettAcceleration<0.1 )
            nettAcceleration = 0.1;

        console.log(nettAcceleration);
        console.log(nettGyro);
        var signal = (1.0/nettAcceleration);
        client.gauge('imu.acceleration', signal*signal);

        if(nettAcceleration < 0.3)
            sendSms("Alert ! We suspect that your dad has fallen.");
        client.gauge('imu.gyro', nettGyro);
    }
    catch(e){
        console.log("Error : " + e);
    }

    res.send("ack");
});

router.get('/location', function(req, res, next) {
    res.json(currentLocation);
});

module.exports = router;
