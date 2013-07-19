var OpenNI = require('..');

var context = OpenNI();

context.startCOMTracking();

context.emit = function() {
  console.log('emitted', arguments);
}

process.on('SIGINT', function() {
  context.close();
  process.exit();  
});
