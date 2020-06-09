'use strict';
const common = require('../common');
const http = require('http');

const server = http.createServer((req, res) => res.flushHeaders());

server.listen(common.mustCall(() => {
  const address = server.address();
  address.host = address.address;
  const req = http.get(address, common.mustCall((res) => {
    res.on('timeout', common.mustCall(() => req.destroy()));
    res.setTimeout(1);
    server.close();
  }));
}));
