// vim: noet

mod api;

static SPN_SHM_FILE:    &str = "testdata/shm1.bin"; //"/spnshm/shm";
static SPN_SOCKET_FILE: &str = "/spnshm/socket";

fn main() {
	/*
	// For cross-checking structure layout between different programming languages.
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
	*/

	let a = api::Api::new(SPN_SHM_FILE).unwrap();

	println!("Number of food items: {}", a.get_food().len());
	println!("Number of segments:   {}", a.get_segments().len());
	println!("Number of bots:       {}", a.get_bot_info().len());

	let f = &a.get_food()[1];

	println!("Location:   {}/{}", f.x, f.y);
	println!("Distance:   {}", f.dist);
	println!("Direction:  {}", f.dir);
	println!("Value:      {}", f.val);
}
