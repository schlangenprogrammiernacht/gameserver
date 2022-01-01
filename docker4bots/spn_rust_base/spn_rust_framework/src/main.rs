// vim: noet
extern crate memmap2;

use std::fs::OpenOptions;
use std::mem::{size_of, transmute};

use memmap2::MmapRaw;

mod ipc;
use crate::ipc::*;

static SPN_SHM_FILE:    &str = "testdata/shm1.bin"; //"/spnshm/shm";
static SPN_SOCKET_FILE: &str = "/spnshm/socket";

fn main() {
	println!("sizeof(IpcSelfInfo)     = {:8}", size_of::<ipc::IpcSelfInfo>());
	println!("sizeof(IpcServerConfig) = {:8}", size_of::<ipc::IpcServerConfig>());
	println!("sizeof(IpcFoodInfo)     = {:8}", size_of::<ipc::IpcFoodInfo>());
	println!("sizeof(IpcBotInfo)      = {:8}", size_of::<ipc::IpcBotInfo>());
	println!("sizeof(IpcSegmentInfo)  = {:8}", size_of::<ipc::IpcSegmentInfo>());
	println!("sizeof(IpcColor)        = {:8}", size_of::<ipc::IpcColor>());
	println!("sizeof(IpcSharedMemory) = {:8}", size_of::<ipc::IpcSharedMemory>());
	println!("sizeof(IpcRequest)      = {:8}", size_of::<ipc::IpcRequest>());
	println!("sizeof(IpcStepResponse) = {:8}", size_of::<ipc::IpcStepResponse>());
	println!("sizeof(IpcResponse)     = {:8}", size_of::<ipc::IpcResponse>());
	println!("sizeof(bool)            = {:8}", size_of::<bool>());

	let file = OpenOptions::new()
		.read(true)
		.write(true)
		.open(SPN_SHM_FILE).unwrap();

	let mmap = MmapRaw::map_raw(&file).unwrap();

	if mmap.len() < size_of::<ipc::IpcSharedMemory>() {
		panic!("Shared memory contains only {} bytes where {} bytes are required.",
			   mmap.len(), size_of::<ipc::IpcSharedMemory>());
	}

	let ipcdata = unsafe {
		&mut *transmute::<*mut u8, *mut ipc::IpcSharedMemory>(mmap.as_mut_ptr())
	};

	println!("Food Count: {}", ipcdata.food_count);
}
