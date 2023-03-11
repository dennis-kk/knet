fn main() {
    println!("cargo:rerun-if-changed=build.rs");
    // Use the `cc` crate to build a C file and statically link it.
    // TODO 条件编译
    cc::Build::new()
        .files(
            vec!["../../knet/address.c",
                 "../../knet/ringbuffer.c",
                 "../../knet/buffer.c",
                 "../../knet/misc.c",
                 "../../knet/list.c",
                 "../../knet/channel.c",
                 "../../knet/logger.c",
                 "../../knet/channel_ref.c",
                 "../../knet/timer.c",
                 "../../knet/stream.c",
                 "../../knet/rb_tree.c",
                 "../../knet/loop_profile.c",
                 "../../knet/loop_balancer.c",
                 "../../knet/loop_select.c",
                 "../../knet/loop_impl.c",
                 "../../knet/loop.c"])
        .include("../../knet")
        .compile("knet");
}