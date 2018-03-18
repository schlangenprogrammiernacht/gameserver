function set_time_quota(secs)
    local st=os.clock()
    function check()
        if os.clock()-st > secs then
            debug.sethook() -- disable hooks
            error("time quota exceeded")
        end
    end
    debug.sethook(check, "", 10000);
end

instruction_counter = 0
function set_instruction_quota(num_events)
    local events_per_check = 1000
    function check()
        instruction_counter = instruction_counter + events_per_check
        if instruction_counter > num_events then
            debug.sethook()
            error("instruction quota exceeded")
        end
    end
    instruction_counter = 0
    debug.sethook(check, "", events_per_check)
end
