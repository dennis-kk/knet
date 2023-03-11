use heapless::spsc::Queue;
use libc::{c_char, c_int, c_uint, c_ulonglong, c_void};
use std::{
    cell::RefCell,
    collections::HashMap,
    ffi::CStr,
    ffi::CString,
    ptr,
    rc::Rc,
    sync::{
        self,
        atomic::{AtomicBool, Ordering},
        Arc, Mutex,
    },
    thread,
};

#[allow(dead_code)]
#[repr(C)]
#[derive(PartialEq)]
/// 管道事件
pub enum ChannelCbEvent {
    ChannelCbEventConnect = 1,         // 连接完成
    ChannelCbEventAccept = 2,          // 管道监听到了新连接请求
    ChannelCbEventRecv = 4,            // 管道有数据可以读
    ChannelCbEventSend = 8,            // 管道发送了字节，保留
    ChannelCbEventClose = 16,          // 管道关闭
    ChannelCbEventTimeout = 32,        // 管道读空闲
    ChannelCbEventConnectTimeout = 64, // 主动发起连接，但连接超时
    ChannelCbEventAcceptFailed,        // Accept失败
    ChannelCbEventAcceptClient,        // 接收到新的连接
    ChannelCbEventConnectFailed,       // 连接失败
}

extern "C" {
    #[allow(dead_code)]
    pub fn knet_loop_create() -> *mut c_void;
    #[allow(dead_code)]
    pub fn knet_loop_destroy(c_loop: *mut c_void);
    #[allow(dead_code)]
    pub fn knet_loop_create_channel(
        c_loop: *mut c_void,
        max_send_list_len: c_uint,
        recv_ring_len: c_uint,
    ) -> *mut c_void;
    #[allow(dead_code)]
    pub fn knet_loop_run_once(c_loop: *mut c_void) -> c_int;
    #[allow(dead_code)]
    pub fn knet_channel_ref_accept(
        c_channel: *mut c_void,
        ip: *const c_char,
        port: c_int,
        backlog: c_int,
    ) -> c_int;
    #[allow(dead_code)]
    pub fn knet_channel_ref_connect(
        c_channel: *mut c_void,
        ip: *const c_char,
        port: c_int,
        timeout: c_int,
    ) -> c_int;
    #[allow(dead_code)]
    pub fn knet_channel_ref_close(c_channel: *mut c_void);
    #[allow(dead_code)]
    pub fn knet_channel_ref_get_stream(c_channel: *mut c_void) -> *mut c_void;
    #[allow(dead_code)]
    pub fn knet_channel_ref_set_cb(
        c_channel: *mut c_void,
        cb: extern "C" fn(*mut c_void, ChannelCbEvent),
    );
    #[allow(dead_code)]
    pub fn knet_channel_ref_get_uuid(c_channel: *mut c_void) -> c_ulonglong;
    #[allow(dead_code)]
    pub fn knet_channel_ref_get_peer_address(c_channel: *mut c_void) -> *mut c_void;
    #[allow(dead_code)]
    pub fn knet_channel_ref_get_local_address(c_channel: *mut c_void) -> *mut c_void;
    #[allow(dead_code)]
    pub fn knet_stream_available(stream: *mut c_void) -> c_int;
    #[allow(dead_code)]
    pub fn knet_stream_eat(stream: *mut c_void, size: c_int) -> c_int;
    #[allow(dead_code)]
    pub fn knet_stream_pop(stream: *mut c_void, buffer: *mut c_void, size: c_int) -> c_int;
    #[allow(dead_code)]
    pub fn knet_stream_push(stream: *mut c_void, buffer: *const c_void, size: c_int) -> c_int;
    #[allow(dead_code)]
    pub fn knet_stream_copy(stream: *mut c_void, buffer: *mut c_void, size: c_int) -> c_int;
    #[allow(dead_code)]
    pub fn address_get_ip(address: *mut c_void) -> *const c_char;
    #[allow(dead_code)]
    pub fn address_get_port(address: *mut c_void) -> c_int;
    #[allow(dead_code)]
    pub fn knet_channel_ref_get_ptr(c_channel: *mut c_void) -> *mut c_void;
    #[allow(dead_code)]
    pub fn knet_channel_ref_set_ptr(c_channel: *mut c_void, data: *mut c_void);
}

/// 管道回调函数类型
type ChannelCallBack = Rc<RefCell<dyn FnMut(&mut Channel, ChannelCbEvent)>>;

/// 网络管道
/// #[derive(Debug)]
pub struct Channel {
    channel_ptr: *mut c_void, // knet网络管道
    close_flag: bool,         // 关闭标志
    cb: ChannelCallBack,      // 管道回调函数
}

/// 实现Channel方法
impl Channel {
    #[allow(dead_code)]
    /// 关闭管道
    pub fn close(self: &mut Channel) {
        if self.close_flag {
            return;
        }
        // 设置关闭标志
        self.close_flag = true;
        unsafe {
            // 关闭管道
            knet_channel_ref_close(self.channel_ptr);
        }
    }

    #[allow(dead_code)]
    /// 检测管道是否关闭
    pub fn is_close(self: &mut Channel) -> bool {
        return self.close_flag;
    }

    #[allow(dead_code)]
    #[doc(hidden)] // 此方法不对外
    fn set_dispose(self: &mut Channel) {
        self.close_flag = true;
    }

    #[allow(dead_code)]
    /// 获取管道UUID
    pub fn get_uuid(self: &mut Channel) -> u64 {
        unsafe {
            return knet_channel_ref_get_uuid(self.channel_ptr);
        }
    }

    #[allow(dead_code)]
    /// 检查管道内字节流数量
    pub fn available(self: &mut Channel) -> i32 {
        if self.close_flag {
            return 0;
        }
        unsafe {
            let stream = knet_channel_ref_get_stream(self.channel_ptr);
            if stream.is_null() {
                return 0;
            } else {
                return knet_stream_available(stream);
            }
        }
    }

    #[allow(dead_code)]
    pub fn set_handler(self: &mut Channel, cb: impl FnMut(&mut Channel, ChannelCbEvent) + 'static) {
        self.cb = Rc::new(RefCell::new(cb));
    }

    #[allow(dead_code)]
    pub fn read(self: &mut Channel, buffer: &mut Vec<u8>, size: i32) -> i32 {
        if self.close_flag {
            return 0;
        }
        unsafe {
            let stream = knet_channel_ref_get_stream(self.channel_ptr);
            if stream.is_null() {
                return 0;
            } else {
                let bytes = knet_stream_available(stream);
                if bytes > size {
                    return 0;
                }
                if 0 == knet_stream_pop(stream, buffer.as_mut_ptr() as *mut c_void, size) {
                    return size;
                } else {
                    return 0;
                }
            }
        }
    }

    #[allow(dead_code)]
    pub fn copy(self: &mut Channel, buffer: &mut Vec<u8>, size: i32) -> i32 {
        if self.close_flag {
            return 0;
        }
        unsafe {
            let stream = knet_channel_ref_get_stream(self.channel_ptr);
            if stream.is_null() {
                return 0;
            } else {
                let bytes = knet_stream_available(stream);
                if bytes > size {
                    return 0;
                }
                if 0 == knet_stream_copy(stream, buffer.as_mut_ptr() as *mut c_void, size) {
                    return size;
                } else {
                    return 0;
                }
            }
        }
    }

    #[allow(dead_code)]
    pub fn write(self: &mut Channel, buffer: &Vec<u8>) -> i32 {
        if self.close_flag {
            return 0;
        }
        let size = buffer.len() as i32;
        unsafe {
            let stream = knet_channel_ref_get_stream(self.channel_ptr);
            if stream.is_null() {
                return 0;
            } else {
                match knet_stream_push(stream, buffer.as_ptr() as *mut c_void, size) {
                    0 => size,
                    _ => 0,
                }
            }
        }
    }

    #[allow(dead_code)]
    pub fn get_local_address(self: &mut Channel) -> (&'static str, i32) {
        if self.close_flag {
            return ("", 0);
        } else {
            unsafe {
                let addr = knet_channel_ref_get_local_address(self.channel_ptr);
                let ip = address_get_ip(addr);
                let port = address_get_port(addr);
                (CStr::from_ptr(ip).to_str().unwrap(), port)
            }
        }
    }

    #[allow(dead_code)]
    pub fn get_peer_address(self: &mut Channel) -> (&'static str, i32) {
        if self.close_flag {
            return ("", 0);
        } else {
            unsafe {
                let addr = knet_channel_ref_get_peer_address(self.channel_ptr);
                let ip = address_get_ip(addr);
                let port = address_get_port(addr);
                (CStr::from_ptr(ip).to_str().unwrap(), port)
            }
        }
    }
}

#[derive(Debug, Clone)]
pub struct Loop {
    loop_ptr: *mut c_void, // 同一线程内共享的网络循环
    close_flag: bool,      // 关闭标志
}

impl Drop for Loop {
    fn drop(&mut self) {
        self.destroy();
    }
}

pub struct CLoopWrapper {
    pub loop_ptr: *mut c_void,                       // 网络循环, 每个线程一个
    pub count: i32,                                  // 单个线程内网络循环数量
    channel_map: HashMap<u64, Rc<RefCell<Channel>>>, // 单个线程内管道表
}

thread_local! {
    // 每个线程内建立一个实例
    static LOOP_GLOBAL_PTR: RefCell<CLoopWrapper> =
        RefCell::new(CLoopWrapper{
            loop_ptr: ptr::null_mut(),
            count: 0,
            channel_map: HashMap::new(),
        });
}

impl Loop {
    /// 建立一个网络循环实例
    #[allow(dead_code)]
    pub fn new() -> Option<Loop> {
        unsafe {
            let loop_ptr = knet_loop_create();
            if loop_ptr.is_null() {
                return None;
            }
            LOOP_GLOBAL_PTR.with(|p| {
                p.borrow_mut().count += 1;
                if !p.borrow_mut().loop_ptr.is_null() {
                    // 已经建立过
                    return Some(Loop {
                        loop_ptr,
                        close_flag: false,
                    });
                } else {
                    // 首次建立
                    p.borrow_mut().loop_ptr = loop_ptr;
                }
                let loop_inst = Loop {
                    loop_ptr,
                    close_flag: false,
                };
                Some(loop_inst)
            })
        }
    }

    /// 销毁一个网络循环实例
    #[allow(dead_code)]
    pub fn destroy(self: &mut Loop) {
        unsafe {
            if self.close_flag {
                return;
            }
            self.close_flag = true;
            LOOP_GLOBAL_PTR.with(|p| {
                p.borrow_mut().channel_map.clear();
                p.borrow_mut().count -= 1;
                if p.borrow_mut().count == 0 {
                    knet_loop_destroy(self.loop_ptr);
                    p.borrow_mut().loop_ptr = ptr::null_mut();
                }
            });
        }
    }

    /// 运行网络循环
    #[allow(dead_code)]
    pub fn tick(self: &Loop) {
        if self.close_flag {
            return;
        }
        unsafe {
            knet_loop_run_once(self.loop_ptr);
        }
    }

    /// 关闭管道
    ///
    ///  # Arguments
    ///
    /// * `chan_id` - 管道ID
    #[allow(dead_code)]
    pub fn close(self: &Loop, chan_id: u64) {
        LOOP_GLOBAL_PTR.with(|p| {
            let channel_mut = match p.borrow_mut().channel_map.get(&chan_id) {
                Some(c) => c.clone(),
                _ => return,
            };
            channel_mut.as_ref().borrow_mut().close();
        });
    }

    /// 发送数据
    ///
    ///  # Arguments
    ///
    /// * `chan_id` - 管道ID
    /// * `data` - 需要发送的数据
    #[allow(dead_code)]
    pub fn send(self: &Loop, chan_id: u64, data: &Vec<u8>) {
        LOOP_GLOBAL_PTR.with(|p| {
            let channel_mut = match p.borrow_mut().channel_map.get(&chan_id) {
                Some(c) => c.clone(),
                _ => return,
            };
            channel_mut.as_ref().borrow_mut().write(&data);
        });
    }

    /// 开启监听
    ///
    /// # Arguments
    ///
    /// * `ip` - IP地址
    /// * `port` - 端口
    /// * `backlog` - backlog
    /// * `cb` - 网络事件回调函数
    #[allow(dead_code)]
    pub fn accept(
        self: &Loop,
        ip: &str,
        port: i32,
        backlog: i32,
        cb: impl FnMut(&mut Channel, ChannelCbEvent) + 'static,
    ) -> Option<Rc<RefCell<Channel>>> {
        if self.close_flag {
            return None;
        }
        unsafe {
            let channel_ptr = knet_loop_create_channel(self.loop_ptr, 1024, 1024);
            if channel_ptr.is_null() {
                return None;
            }
            knet_channel_ref_set_cb(channel_ptr, Loop::_acceptor_cb);
            let accept_chan_id = knet_channel_ref_get_uuid(channel_ptr);
            // 设置Acceptor管道ID
            knet_channel_ref_set_ptr(channel_ptr, accept_chan_id as *mut c_void);
            let ip_ptr = CString::new(ip).unwrap();
            let e = knet_channel_ref_accept(channel_ptr, ip_ptr.as_ptr(), port, backlog);
            if e == 0 {
                // 监听成功
                let channel_sptr = Rc::new(RefCell::new(Channel {
                    channel_ptr,
                    close_flag: false,
                    cb: Rc::new(RefCell::new(cb)),
                }));
                LOOP_GLOBAL_PTR.with(|p| {
                    p.borrow_mut()
                        .channel_map
                        .insert(accept_chan_id, channel_sptr.clone());
                    let mut _cb = (*channel_sptr.clone().as_ref().borrow_mut()).cb.clone();
                    (_cb.as_ref().borrow_mut())(
                        &mut *channel_sptr.clone().as_ref().borrow_mut(),
                        ChannelCbEvent::ChannelCbEventAccept,
                    );
                    Some(channel_sptr.clone())
                })
            } else {
                // 监听失败
                let channel_sptr = Rc::new(RefCell::new(Channel {
                    channel_ptr,
                    close_flag: true,
                    cb: Rc::new(RefCell::new(cb)),
                }));
                LOOP_GLOBAL_PTR.with(|p| {
                    p.borrow_mut()
                        .channel_map
                        .insert(accept_chan_id, channel_sptr.clone());
                    let mut _cb = (*channel_sptr.clone().as_ref().borrow_mut()).cb.clone();
                    (_cb.as_ref().borrow_mut())(
                        &mut *channel_sptr.clone().as_ref().borrow_mut(),
                        ChannelCbEvent::ChannelCbEventAcceptFailed,
                    );
                    knet_channel_ref_close(channel_ptr);
                    None
                })
            }
        }
    }

    /// 连接远程服务
    ///
    /// # Arguments
    ///
    /// * `ip` - IP地址
    /// * `port` - 端口
    /// * `timeout` - 连接超时(秒)
    /// * `cb` - 网络事件回调函数
    #[allow(dead_code)]
    pub fn connect(
        self: &Loop,
        ip: &str,
        port: i32,
        timeout: i32,
        cb: impl FnMut(&mut Channel, ChannelCbEvent) + 'static,
    ) -> Option<Rc<RefCell<Channel>>> {
        unsafe {
            let channel_ptr = knet_loop_create_channel(self.loop_ptr, 1024, 1024);
            if channel_ptr.is_null() {
                return None;
            }
            knet_channel_ref_set_cb(channel_ptr, Loop::_client_cb);
            let ip_ptr = CString::new(ip).unwrap();
            let e = knet_channel_ref_connect(channel_ptr, ip_ptr.as_ptr(), port, timeout);
            if e == 0 {
                let channel_sptr = Rc::new(RefCell::new(Channel {
                    channel_ptr,
                    close_flag: false,
                    cb: Rc::new(RefCell::new(cb)),
                }));
                // 添加到map
                LOOP_GLOBAL_PTR.with(|p| {
                    p.borrow_mut()
                        .channel_map
                        .insert(knet_channel_ref_get_uuid(channel_ptr), channel_sptr.clone());
                    Some(channel_sptr)
                })
            } else {
                let channel_sptr = Rc::new(RefCell::new(Channel {
                    channel_ptr,
                    close_flag: true,
                    cb: Rc::new(RefCell::new(cb)),
                }));
                // 添加到map
                LOOP_GLOBAL_PTR.with(|p| {
                    p.borrow_mut()
                        .channel_map
                        .insert(knet_channel_ref_get_uuid(channel_ptr), channel_sptr.clone());
                    let mut _cb = (*channel_sptr.clone().as_ref().borrow_mut()).cb.clone();
                    (_cb.as_ref().borrow_mut())(
                        &mut *channel_sptr.clone().as_ref().borrow_mut(),
                        ChannelCbEvent::ChannelCbEventConnectFailed,
                    );
                    knet_channel_ref_close(channel_ptr);
                    None
                })
            }
        }
    }

    extern "C" fn _client_cb(_channel_ptr: *mut c_void, _e: ChannelCbEvent) {
        unsafe {
            let chan_id = knet_channel_ref_get_uuid(_channel_ptr);
            let channel_mut =
                LOOP_GLOBAL_PTR.with(|p| match p.borrow_mut().channel_map.get(&chan_id) {
                    Some(c) => c.clone(),
                    _ => panic!("Client not found, channel UUID {}", chan_id),
                });
            let cb_ref = &mut channel_mut.as_ref().borrow_mut().cb.clone();
            let mut cb = cb_ref.as_ref().borrow_mut();
            match _e {
                ChannelCbEvent::ChannelCbEventClose => {
                    // 设置关闭标志，防止close在回调内调用
                    channel_mut.as_ref().borrow_mut().set_dispose();
                    // 管道关闭
                    (cb)(
                        &mut channel_mut.as_ref().borrow_mut(),
                        ChannelCbEvent::ChannelCbEventClose,
                    );
                    LOOP_GLOBAL_PTR.with(|p| {
                        p.borrow_mut().channel_map.remove(&chan_id);
                    });
                }
                ChannelCbEvent::ChannelCbEventConnect => {
                    // 连接成功
                    (cb)(
                        &mut channel_mut.as_ref().borrow_mut(),
                        ChannelCbEvent::ChannelCbEventConnect,
                    );
                }
                ChannelCbEvent::ChannelCbEventRecv => {
                    // 有数据可读
                    (cb)(
                        &mut channel_mut.as_ref().borrow_mut(),
                        ChannelCbEvent::ChannelCbEventRecv,
                    );
                }
                ChannelCbEvent::ChannelCbEventConnectTimeout => {
                    // 连接超时
                    (cb)(
                        &mut channel_mut.as_ref().borrow_mut(),
                        ChannelCbEvent::ChannelCbEventConnectTimeout,
                    );
                }
                ChannelCbEvent::ChannelCbEventAccept => todo!(),
                ChannelCbEvent::ChannelCbEventSend => todo!(),
                ChannelCbEvent::ChannelCbEventTimeout => todo!(),
                ChannelCbEvent::ChannelCbEventAcceptFailed => todo!(),
                ChannelCbEvent::ChannelCbEventAcceptClient => todo!(),
                ChannelCbEvent::ChannelCbEventConnectFailed => todo!(),
            }
        }
    }

    extern "C" fn _acceptor_cb(_channel_ptr: *mut c_void, _e: ChannelCbEvent) {
        let mut _channel_map = LOOP_GLOBAL_PTR.with(|p| p.borrow_mut().channel_map.clone());
        let cb;
        unsafe {
            // 获取Acceptor channel UUID
            let accept_chan_id = knet_channel_ref_get_ptr(_channel_ptr) as u64;
            // 获取当前客户端管道UUID
            let chan_id = knet_channel_ref_get_uuid(_channel_ptr);
            match _e {
                ChannelCbEvent::ChannelCbEventAccept => {
                    // 获取Acceptor channel
                    let _ = match _channel_map.get(&accept_chan_id) {
                        Some(c) => c.as_ref().borrow().channel_ptr,
                        _ => panic!("Acceptor not found, channle UUID {}", accept_chan_id),
                    };
                    // 设置客户端管道回调
                    knet_channel_ref_set_cb(_channel_ptr, Loop::_client_cb);
                    cb = _channel_map
                        .get(&accept_chan_id)
                        .unwrap()
                        .as_ref()
                        .borrow()
                        .cb
                        .clone();
                    // 客户端管道加入map
                    let _ = _channel_map.insert(
                        chan_id,
                        Rc::new(RefCell::new(Channel {
                            channel_ptr: _channel_ptr,
                            close_flag: false,
                            cb: cb.clone(), // 设置Acceptor的回调
                        })),
                    );
                    // 调用Acceptor回调
                    let accept_channel =
                        _channel_map.get(&accept_chan_id).unwrap().as_ref().borrow();
                    (accept_channel.cb.as_ref().borrow_mut())(
                        &mut Channel {
                            channel_ptr: _channel_ptr,
                            close_flag: false,
                            cb: accept_channel.cb.clone(), // 设置Acceptor的回调
                        },
                        ChannelCbEvent::ChannelCbEventAcceptClient,
                    );
                }
                ChannelCbEvent::ChannelCbEventConnect => todo!(),
                ChannelCbEvent::ChannelCbEventRecv => todo!(),
                ChannelCbEvent::ChannelCbEventSend => todo!(),
                ChannelCbEvent::ChannelCbEventClose => todo!(),
                ChannelCbEvent::ChannelCbEventTimeout => todo!(),
                ChannelCbEvent::ChannelCbEventConnectTimeout => todo!(),
                ChannelCbEvent::ChannelCbEventAcceptFailed => todo!(),
                ChannelCbEvent::ChannelCbEventAcceptClient => todo!(),
                ChannelCbEvent::ChannelCbEventConnectFailed => todo!(),
            };
        }
    }
}

#[derive(Debug, Clone)]
struct NetEventListenReq {
    ip: String,
    port: i32,
}

#[derive(Debug, Clone)]
struct NetEventListenAck {
    ip: String,
    port: i32,
    success: bool,
}

#[derive(Debug, Clone)]
struct NetEventAccept {
    chan_id: u64,
}

#[derive(Debug, Clone)]
struct NetEventConnectReq {
    ip: String,
    port: i32,
}

#[derive(Debug, Clone)]
struct NetEventConnectAck {
    ip: String,
    port: i32,
    success: bool,
    chan_id: u64,
}

#[derive(Debug, Clone)]
struct NetEventSendNtf {
    _chan_id: u64,
    _data: Vec<u8>,
}

#[derive(Debug, Clone)]
struct NetEventCloseReq {
    chan_id: u64,
}

#[derive(Debug, Clone)]
struct NetEventCloseNtf {
    chan_id: u64,
}

#[allow(dead_code)]
#[derive(Debug, Clone)]
struct NetEventDataNtf {
    chan_id: u64,
    data: Vec<u8>,
}

#[derive(Debug, Clone)]
enum NetEvent {
    EventListenReq(NetEventListenReq),
    EventListenAck(NetEventListenAck),
    EventConnectReq(NetEventConnectReq),
    EventConnectAck(NetEventConnectAck),
    EventSendNtf(NetEventSendNtf),
    EventCloseReq(NetEventCloseReq),
    EventCloseNtf(NetEventCloseNtf),
    EventAccept(NetEventAccept),
    EventDataNtf(NetEventDataNtf),
}

#[allow(dead_code)]
pub struct KnetNetwork {
    running: sync::Arc<AtomicBool>,
    worker: Option<thread::JoinHandle<()>>,
    net_to_worker_queue: Arc<Mutex<Queue<NetEvent, 1024>>>,
    worker_to_net_queue: Arc<Mutex<Queue<NetEvent, 1024>>>,
    on_accept_cb: Rc<RefCell<dyn FnMut(&KnetNetwork, &str, i32, bool)>>,
    on_connect_cb: Rc<RefCell<dyn FnMut(&KnetNetwork, u64, &str, i32, bool)>>,
    on_read_cb: Rc<RefCell<dyn FnMut(&KnetNetwork, u64, &Vec<u8>)>>,
    on_close_cb: Rc<RefCell<dyn FnMut(&KnetNetwork, u64)>>,
    on_client_cb: Rc<RefCell<dyn FnMut(&KnetNetwork, u64)>>,
}

impl Drop for KnetNetwork {
    fn drop(&mut self) {
        self.stop();
    }
}

impl KnetNetwork {
    /// 建立一个网络实例
    #[allow(dead_code)]
    pub fn new() -> Arc<Mutex<KnetNetwork>> {
        Arc::new(Mutex::new(KnetNetwork {
            running: sync::Arc::new(AtomicBool::new(false)),
            net_to_worker_queue: Arc::new(Mutex::new(Queue::new())),
            worker_to_net_queue: Arc::new(Mutex::new(Queue::new())),
            worker: None,
            on_accept_cb: Rc::new(RefCell::new(KnetNetwork::_default_on_accept_cb)),
            on_connect_cb: Rc::new(RefCell::new(KnetNetwork::_default_on_connect_cb)),
            on_read_cb: Rc::new(RefCell::new(KnetNetwork::_default_on_read_cb)),
            on_close_cb: Rc::new(RefCell::new(KnetNetwork::_default_on_close_cb)),
            on_client_cb: Rc::new(RefCell::new(KnetNetwork::_default_on_client_cb)),
        }))
    }

    #[allow(dead_code)]
    pub fn on_accept(&mut self, cb: impl FnMut(&KnetNetwork, &str, i32, bool) + 'static) {
        self.on_accept_cb = Rc::new(RefCell::new(cb));
    }

    #[allow(dead_code)]
    pub fn on_connect(&mut self, cb: impl FnMut(&KnetNetwork, u64, &str, i32, bool) + 'static) {
        self.on_connect_cb = Rc::new(RefCell::new(cb));
    }

    #[allow(dead_code)]
    pub fn on_data(&mut self, cb: impl FnMut(&KnetNetwork, u64, &Vec<u8>) + 'static) {
        self.on_read_cb = Rc::new(RefCell::new(cb));
    }

    #[allow(dead_code)]
    pub fn on_close(&mut self, cb: impl FnMut(&KnetNetwork, u64) + 'static) {
        self.on_close_cb = Rc::new(RefCell::new(cb));
    }

    #[allow(dead_code)]
    pub fn on_client(&mut self, cb: impl FnMut(&KnetNetwork, u64) + 'static) {
        self.on_client_cb = Rc::new(RefCell::new(cb));
    }

    #[allow(dead_code)]
    pub fn start(&mut self) {
        if self.running.load(Ordering::SeqCst) {
            // 已经启动
            return;
        }
        // 设置工作线程运行标志为true
        self.running.store(true, Ordering::SeqCst);
        let running = self.running.clone();
        let net_to_worker_queue = self.net_to_worker_queue.clone();
        let worker_to_net_queue = self.worker_to_net_queue.clone();
        // 启动网络工作线程
        self.worker = Some(thread::spawn(move || {
            // 每个线程独立的网络循环
            let net_loop = Loop::new().unwrap();
            while running.load(Ordering::SeqCst) {
                // 工作线程运行网络循环
                net_loop.tick();
                // 处理事件
                KnetNetwork::_process_worker_event(
                    net_to_worker_queue.clone(),
                    worker_to_net_queue.clone(),
                    &net_loop,
                );
            }
            // 主动退出
            running.store(false, Ordering::SeqCst);
        }));
    }

    #[allow(dead_code)]
    pub fn stop(&mut self) {
        if !self.running.load(Ordering::SeqCst) {
            return;
        }
        // 设置运行标志为false
        self.running.store(false, Ordering::SeqCst);
        // 等待工作线程退出
        self.worker
            .take()
            .expect("Non-running worker thread")
            .join()
            .expect("Join worker thread failed");
    }

    #[allow(dead_code)]
    pub fn listen(self: &mut KnetNetwork, ip: String, port: i32) {
        KnetNetwork::_notify_worker(
            self.net_to_worker_queue.clone(),
            NetEvent::EventListenReq(NetEventListenReq {
                ip: ip.to_string(),
                port,
            }),
        );
    }

    #[allow(dead_code)]
    pub fn connect(self: &KnetNetwork, ip: String, port: i32) {
        KnetNetwork::_notify_worker(
            self.net_to_worker_queue.clone(),
            NetEvent::EventConnectReq(NetEventConnectReq {
                ip: ip.to_string(),
                port: port,
            }),
        );
    }

    #[allow(dead_code)]
    pub fn tick(self: &KnetNetwork) {
        self._process_net_event();
    }

    #[allow(dead_code)]
    pub fn send(self: &KnetNetwork, chan_id: u64, data: &Vec<u8>) {
        KnetNetwork::_notify_worker(
            self.net_to_worker_queue.clone(),
            NetEvent::EventSendNtf(NetEventSendNtf {
                _chan_id: chan_id,
                _data: data.to_vec(),
            }),
        );
    }

    #[allow(dead_code)]
    pub fn close(self: &KnetNetwork, chan_id: u64) {
        KnetNetwork::_notify_worker(
            self.net_to_worker_queue.clone(),
            NetEvent::EventCloseReq(NetEventCloseReq { chan_id: chan_id }),
        );
    }

    fn _default_on_accept_cb(_self: &KnetNetwork, _1: &str, _2: i32, _3: bool) {}
    fn _default_on_connect_cb(_self: &KnetNetwork, _0: u64, _1: &str, _2: i32, _3: bool) {}
    fn _default_on_read_cb(_self: &KnetNetwork, _1: u64, _2: &Vec<u8>) {}
    fn _default_on_close_cb(_self: &KnetNetwork, _1: u64) {}
    fn _default_on_client_cb(_self: &KnetNetwork, _1: u64) {}

    fn _notify_main(queue: Arc<Mutex<Queue<NetEvent, 1024>>>, e: NetEvent) {
        queue.lock().unwrap().enqueue(e).unwrap();
    }

    fn _notify_worker(queue: Arc<Mutex<Queue<NetEvent, 1024>>>, e: NetEvent) {
        queue.lock().unwrap().enqueue(e).unwrap();
    }

    fn _process_listen_in_worker(
        net_loop: &Loop,
        req: &NetEventListenReq,
        net_queue: Arc<Mutex<Queue<NetEvent, 1024>>>,
    ) {
        net_loop.accept(
            &req.ip,
            req.port,
            512,
            move |_chan: &mut Channel, _e: ChannelCbEvent| {
                match _e {
                    ChannelCbEvent::ChannelCbEventRecv => {
                        let size = _chan.available();
                        let mut data = Vec::with_capacity(size.try_into().unwrap());
                        let uuid = _chan.get_uuid();
                        match _chan.read(&mut data, size) {
                            0 => return,
                            _ => unsafe { data.set_len(size.try_into().unwrap()) },
                        }
                        KnetNetwork::_notify_main(
                            net_queue.clone(),
                            NetEvent::EventDataNtf(NetEventDataNtf {
                                chan_id: uuid,
                                data,
                            }),
                        );
                    }
                    ChannelCbEvent::ChannelCbEventAccept => {
                        // 发送NetEventListenAck，成功
                        KnetNetwork::_notify_main(
                            net_queue.clone(),
                            NetEvent::EventListenAck(NetEventListenAck {
                                ip: _chan.get_local_address().0.to_owned(),
                                port: _chan.get_local_address().1,
                                success: true,
                            }),
                        );
                    }
                    ChannelCbEvent::ChannelCbEventAcceptFailed => {
                        // 发送NetEventListenAck, 失败
                        KnetNetwork::_notify_main(
                            net_queue.clone(),
                            NetEvent::EventListenAck(NetEventListenAck {
                                ip: _chan.get_local_address().0.to_owned(),
                                port: _chan.get_local_address().1,
                                success: false,
                            }),
                        );
                    }
                    ChannelCbEvent::ChannelCbEventAcceptClient => {
                        // 接收到新的连接
                        // TODO ip, port
                        KnetNetwork::_notify_main(
                            net_queue.clone(),
                            NetEvent::EventAccept(NetEventAccept {
                                chan_id: _chan.get_uuid(),
                            }),
                        );
                    }
                    ChannelCbEvent::ChannelCbEventClose => {
                        // 连接关闭
                        KnetNetwork::_notify_main(
                            net_queue.clone(),
                            NetEvent::EventCloseNtf(NetEventCloseNtf {
                                chan_id: _chan.get_uuid(),
                            }),
                        );
                    }
                    _ => (),
                };
            },
        );
    }

    fn _process_connect_in_worker(
        net_loop: &Loop,
        req: &NetEventConnectReq,
        net_queue: Arc<Mutex<Queue<NetEvent, 1024>>>,
    ) {
        net_loop.connect(
            &req.ip,
            req.port,
            512,
            move |_chan: &mut Channel, _e: ChannelCbEvent| {
                match _e {
                    ChannelCbEvent::ChannelCbEventConnect => {
                        // 连接成功
                        KnetNetwork::_notify_main(
                            net_queue.clone(),
                            NetEvent::EventConnectAck(NetEventConnectAck {
                                chan_id: _chan.get_uuid(),
                                ip: _chan.get_local_address().0.to_owned(),
                                port: _chan.get_local_address().1,
                                success: true,
                            }),
                        );
                    }
                    ChannelCbEvent::ChannelCbEventConnectFailed => {
                        // 连接失败
                        KnetNetwork::_notify_main(
                            net_queue.clone(),
                            NetEvent::EventConnectAck(NetEventConnectAck {
                                chan_id: _chan.get_uuid(),
                                ip: _chan.get_local_address().0.to_owned(),
                                port: _chan.get_local_address().1,
                                success: false,
                            }),
                        );
                    }
                    ChannelCbEvent::ChannelCbEventRecv => {
                        // 网络数据到来
                        let size = _chan.available();
                        let mut data = Vec::with_capacity(size.try_into().unwrap());
                        let uuid = _chan.get_uuid();
                        match _chan.read(&mut data, size) {
                            0 => return,
                            _ => unsafe { data.set_len(size.try_into().unwrap()) },
                        }
                        KnetNetwork::_notify_main(
                            net_queue.clone(),
                            NetEvent::EventDataNtf(NetEventDataNtf {
                                chan_id: uuid,
                                data,
                            }),
                        );
                    }
                    ChannelCbEvent::ChannelCbEventClose => {
                        // 连接关闭
                        KnetNetwork::_notify_main(
                            net_queue.clone(),
                            NetEvent::EventCloseNtf(NetEventCloseNtf {
                                chan_id: _chan.get_uuid(),
                            }),
                        );
                    }
                    ChannelCbEvent::ChannelCbEventConnectTimeout => {
                        // 连接失败
                        KnetNetwork::_notify_main(
                            net_queue.clone(),
                            NetEvent::EventConnectAck(NetEventConnectAck {
                                chan_id: _chan.get_uuid(),
                                ip: _chan.get_local_address().0.to_owned(),
                                port: _chan.get_local_address().1,
                                success: false,
                            }),
                        );
                    }
                    _ => (),
                };
            },
        );
    }

    fn _process_worker_event(
        net_to_worker_queue: Arc<Mutex<Queue<NetEvent, 1024>>>,
        worker_to_net_queue: Arc<Mutex<Queue<NetEvent, 1024>>>,
        net_loop: &Loop,
    ) {
        loop {
            match net_to_worker_queue.lock().unwrap().dequeue() {
                Some(c) => {
                    match c {
                        NetEvent::EventListenReq(req) => {
                            KnetNetwork::_process_listen_in_worker(
                                net_loop,
                                &req,
                                worker_to_net_queue.clone(),
                            );
                        }
                        NetEvent::EventConnectReq(req) => {
                            KnetNetwork::_process_connect_in_worker(
                                net_loop,
                                &req,
                                worker_to_net_queue.clone(),
                            );
                        }
                        NetEvent::EventCloseReq(req) => {
                            // 关闭连接
                            net_loop.close(req.chan_id);
                            // 关闭通知
                            KnetNetwork::_notify_main(
                                worker_to_net_queue.clone(),
                                NetEvent::EventCloseNtf(NetEventCloseNtf {
                                    chan_id: req.chan_id,
                                }),
                            );
                        }
                        NetEvent::EventSendNtf(ntf) => {
                            // 发送数据
                            net_loop.send(ntf._chan_id, &ntf._data);
                        }
                        NetEvent::EventListenAck(_) => todo!(),
                        NetEvent::EventConnectAck(_) => todo!(),
                        NetEvent::EventCloseNtf(_) => todo!(),
                        NetEvent::EventAccept(_) => todo!(),
                        NetEvent::EventDataNtf(_) => todo!(),
                    }
                }
                None => return, // 没有事件需要处理
            };
        }
    }

    fn _process_net_event(&self) {
        loop {
            match self.worker_to_net_queue.lock().unwrap().dequeue() {
                Some(c) => match c {
                    NetEvent::EventListenAck(ack) => {
                        (*self.on_accept_cb).borrow_mut()(&self, &ack.ip, ack.port, ack.success);
                    }
                    NetEvent::EventConnectAck(ack) => {
                        (*self.on_connect_cb).borrow_mut()(
                            &self,
                            ack.chan_id,
                            &ack.ip,
                            ack.port,
                            ack.success,
                        );
                    }
                    NetEvent::EventCloseNtf(ntf) => {
                        (*self.on_close_cb).borrow_mut()(&self, ntf.chan_id);
                    }
                    NetEvent::EventAccept(ntf) => {
                        (*self.on_client_cb).borrow_mut()(&self, ntf.chan_id);
                    }
                    NetEvent::EventDataNtf(ntf) => {
                        (*self.on_read_cb).borrow_mut()(&self, ntf.chan_id, &ntf.data);
                    }
                    NetEvent::EventListenReq(_) => todo!(),
                    NetEvent::EventConnectReq(_) => todo!(),
                    NetEvent::EventSendNtf(_) => todo!(),
                    NetEvent::EventCloseReq(_) => todo!(),
                },
                None => return, // 没有事件需要处理
            }
        }
    }
}
