var express = require('express');
var router = express.Router();
var gps = null;
var currentLocation = {
    lat : 1.3147308,
    long : 103.8470128
};


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

router.get('/location', function(req, res, next) {
    res.json(currentLocation);
});

module.exports = router;
