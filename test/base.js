var OpenNI = require('..');
var test = require('tap').test;

test('Constructs', function(t) {
  var context = OpenNI();
  t.ok(context);
  t.end();
});

test('Initializes and shuts down', function(t) {
  var context = OpenNI();
  context.init();
  context.close();
  t.end();
});