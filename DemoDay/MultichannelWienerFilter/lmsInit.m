function lms = lmsInit()
%LMSINIT Summary of this function goes here
%   Detailed explanation goes here
    lms = dsp.LMSFilter; % LMS filter H -> updates weights based on computed signal
    lms.StepSize = 1;
end

