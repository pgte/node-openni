#ifndef NODE_OPENNI_CONF_H
#define NODE_OPENNI_CONF_H

namespace nodeopenni {
  const char * XMLConf = "<OpenNI>"
  "<Licenses>\n"
  "  <License vendor=\"PrimeSense\" key=\"0KOIk2JeIBYClPWVnMoRKn5cdY4=\" />\n"
  "</Licenses>\n"
  "<Log writeToConsole=\"true\" writeToFile=\"false\">\n"
  "  <LogLevel value=\"1\"/>\n"
  "  <Masks>\n"
  "    <Mask name=\"ALL\" on=\"true\"/>\n"
  "  </Masks>\n"
  "  <Dumps>\n"
  "  </Dumps>\n"
  "</Log>\n"
  "<ProductionNodes>\n"
  "  <Node type=\"Depth\" name=\"Depth1\">\n"
  "    <Configuration>\n"
  "      <MapOutputMode xRes=\"640\" yRes=\"480\" FPS=\"30\"/>\n"
  "      <Mirror on=\"true\"/>\n"
  "    </Configuration>\n"
  "  </Node>\n"
  "</ProductionNodes>\n"
"</OpenNI>\n";
}
#endif