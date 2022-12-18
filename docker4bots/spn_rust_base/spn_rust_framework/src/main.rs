// vim: noet

/*!
 * Welcome to the Rust framework documentation for Schlangenprogrammiernacht players!
 *
 * This documentation will guide you while you program your Schlangenprogrammiernacht (SPN) bot in Rust.
 *
 * The code you write in the webinterface will replace the [`usercode`] module, which must provide
 * the [`usercode::init()`] and [`usercode::step()`] functions. These will be called by the
 * framework when requested to do so by the gameserver.
 *
 * We recommend that you start by reading the documentation of the [`api::Api`] struct,
 * which provides safe access to the data provided by the gameserver.
 */

extern crate uds;

#[macro_use]
extern crate num_derive;
extern crate num;

use num::FromPrimitive;

use uds::UnixSeqpacketConn;

use std::mem::{size_of, transmute};

pub mod api;
use api::ipc::{IpcRequestType, IpcResponse, IpcResponseType};

pub mod usercode;
use usercode::{init, step};

static SPN_SHM_FILE: &str = "/spnshm/shm";
static SPN_SOCKET_FILE: &str = "/spnshm/socket";

fn mainloop(mut api: api::Api, socket: UnixSeqpacketConn) {
    let mut running = true;
    let mut rxbuf = [0u8; size_of::<api::ipc::IpcRequest>()];

    while running {
        // receive messages from the Gameserver
        let (len, _truncated) = socket.recv(&mut rxbuf).unwrap();

        // length check
        if len == 0 {
            println!("Socket connection terminated by server.");
            break;
        } else if len < rxbuf.len() {
            println!("Error: packet is too short! Expected {} bytes, but received only {}. Packet ignored.",
			         rxbuf.len(), len);
            continue;
        }

        // check whether the data contains a valid enum value
        let request_value = unsafe { *transmute::<*const u8, *const u32>(rxbuf.as_ptr()) };

        let request_type;

        match api::ipc::IpcRequestType::from_u32(request_value) {
            Some(x) => request_type = x,
            None => {
                println!("Request type cannot be decoded!");
                continue;
            }
        }

        /*
        // reinterpret the received data as struct IpcRequest
        let request: &api::ipc::IpcRequest = unsafe {
            & *transmute::<*const u8, *const api::ipc::IpcRequest>(rxbuf.as_ptr())
        };
        */

        let angle: f32;
        let boost: bool;

        // execute the user functions corresponding to the request type
        match request_type {
            IpcRequestType::Init => {
                running = init(&mut api);
                angle = 0.0;
                boost = false;
            }
            IpcRequestType::Step => {
                // unfortunately, destructuring is not stable yet.
                let (tmp_running, tmp_angle, tmp_boost) = step(&mut api);
                running = tmp_running;
                angle = tmp_angle;
                boost = tmp_boost;
            }
        }

        // build the response structure
        let response = IpcResponse {
            response_type: match running {
                true => IpcResponseType::Ok,
                false => IpcResponseType::Error,
            },
            data: api::ipc::ResponseData {
                step: api::ipc::IpcStepResponse {
                    delta_angle: angle,
                    boost: boost,
                },
            },
        };

        // reinterpret the response structure as byte array
        let txdata: &[u8] = unsafe {
            std::slice::from_raw_parts(
                (&response as *const IpcResponse) as *const u8,
                std::mem::size_of::<IpcResponse>(),
            )
        };

        // send the result packet
        socket.send(txdata).unwrap();
    }
}

fn main() {
    /*
    // For cross-checking structure layout between different programming languages.
    println!("sizeof(IpcSelfInfo)     = {:8}", size_of::<api::ipc::IpcSelfInfo>());
    println!("sizeof(IpcServerConfig) = {:8}", size_of::<api::ipc::IpcServerConfig>());
    println!("sizeof(IpcFoodInfo)     = {:8}", size_of::<api::ipc::IpcFoodInfo>());
    println!("sizeof(IpcBotInfo)      = {:8}", size_of::<api::ipc::IpcBotInfo>());
    println!("sizeof(IpcSegmentInfo)  = {:8}", size_of::<api::ipc::IpcSegmentInfo>());
    println!("sizeof(IpcColor)        = {:8}", size_of::<api::ipc::IpcColor>());
    println!("sizeof(IpcSharedMemory) = {:8}", size_of::<api::ipc::IpcSharedMemory>());
    println!("sizeof(IpcRequest)      = {:8}", size_of::<api::ipc::IpcRequest>());
    println!("sizeof(IpcStepResponse) = {:8}", size_of::<api::ipc::IpcStepResponse>());
    println!("sizeof(IpcResponse)     = {:8}", size_of::<api::ipc::IpcResponse>());
    println!("sizeof(bool)            = {:8}", size_of::<bool>());
    return;
    */

    let a = api::Api::new(SPN_SHM_FILE).unwrap();

    let conn = UnixSeqpacketConn::connect(SPN_SOCKET_FILE).unwrap();

    mainloop(a, conn);
}
