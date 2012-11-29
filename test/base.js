var OpenNI = require('..');
var test = require('tap').test;

test('Initializes and shuts down', function(t) {
  var context = OpenNI();
  setTimeout(function() {
    context.close();
    t.end();
  }, 10000);
});