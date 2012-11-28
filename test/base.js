var OpenNI = require('..');
var test = require('tap').test;

test('Initializes and shuts down', function(t) {
  var context = OpenNI();
  context.close();
  t.end();
});