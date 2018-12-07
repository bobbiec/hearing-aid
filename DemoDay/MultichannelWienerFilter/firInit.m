function fir = firInit()
%LMSINIT Summary of this function goes here
%   Detailed explanation goes here
    fir = dsp.FIRFilter('NumeratorSource', 'Input port', ...
                        'InitialConditions', 1);
end

