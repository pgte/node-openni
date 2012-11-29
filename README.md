# node-openni

OpenNI bindings in Node.js.

Currently only supports one user, even though several users is easy to achieve with some small mods.

# Install

* Install libusb from http://www.libusb.org/
* Install OpenNI from https://github.com/OpenNI/OpenNI

Fork this repo.

```bash
$ cd node-openni
$ node-gyp configure build
```

# Test

Plug in your kinect.

Run:

```bash
$ node test/base
```

Stay in the surrender position in front of the camera, a couple of meters away.
After you're synced you should see 

# Use

```js
var OpenNI = require('openni');

var context = OpenNI();

[
  "head",
  "neck",
  "torso",
  "waist",
  "left_collar",
  "left_shoulder",
  "left_elbow",
  "left_wrist",
  "left_hand",
  "left_fingertip",
  "right_collar",
  "right_shoulder",
  "right_elbow",
  "right_wrist",
  "right_hand",
  "right_fingertip",
  "left_hip",
  "left_knee",
  "left_ankle",
  "left_foot",
  "right_hip",
  "right_knee",
  "right_ankle",
  "right_foot"
].forEach(function(jointName) {
  
  context.on(jointName, function(x, y, z) {
    cconsole.log(jointName + ' moved to (%d, %d, %d)', x, y, z);
  });

});

// Close the context to exit

process.on('SIGINT', function() {
  context.close();
  process.exit();  
});

```