function cb = circularBufferInit(size)
%CIRCULARBUFFERINIT Summary of this function goes here
%   Detailed explanation goes here
    % cb = circularBuffer(zeros(1, size));
    % cb(:) = 0; % weird init thing to get the right size
    
    cb = complex(zeros(1, size), 0); % hack to generate coder code
end

