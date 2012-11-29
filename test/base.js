var OpenNI = require('..');

var context = OpenNI();
process.on('SIGINT', function() {
  context.close();
  process.exit();  
});
