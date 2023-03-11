# knet

A TCP multi-threaded library.

# sample

## Listen and connect

```rust
let net = knet::KnetNetwork::new();
net.lock().unwrap().start();
net.lock().unwrap().on_accept(
    |_network: &knet::KnetNetwork, _ip: &str, _port: i32, _success: bool| {
        // Listen failed or successfully
    },
);
net.lock().unwrap().on_connect(
    |_network: &knet::KnetNetwork, _chan_id: u64, _ip: &str, _port: i32, _success: bool| {
        // Connect failed or successfully
    },
);
net.lock().unwrap().listen(String::from("127.0.0.1"), 12345);
net.lock().unwrap().connect(String::from("127.0.0.1"), 12345);
//
// Run tick in main loop every frame
//
net.lock().unwrap().tick();
```