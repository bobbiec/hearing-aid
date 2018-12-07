function filtered_output = mwf(audio, FRAME_SIZE, P, VAD_cst_param)
% Parameters:
% audio - matrix of double - columns are channels, rows are time
% FRAME_SIZE - int
% P - double, parameter
% VAD_cst_param - vad parameters
%
% Returns:
% filtered_output - matrix of same size as audio
filtered_output = zeros(size(audio));

% initialize circular buffers
noise_buf = circularBufferInit(FRAME_SIZE/2*10);
nb_read_i = 1;
nb_write_i = 1;

speech_buf = circularBufferInit(FRAME_SIZE/2*10);
sb_read_i = 1;
sb_write_i = 1;

% initialize filters
lmsFilter = dsp.LMSFilter; % LMS filter H -> updates weights based on computed signal
firFilter = dsp.FIRFilter('NumeratorSource', 'Input port', ...
                          'InitialConditions', 1);

MCLT_FRAME = FRAME_SIZE/2;

generated_bands = complex(zeros(MCLT_FRAME, 2), 0);

num_frames = size(audio, 1) / FRAME_SIZE;  % non-overlapping
num_windows = num_frames * 2;
for i=0:num_windows-2
%     tic;
    start_i = 1 + FRAME_SIZE*i/2;
    end_i = FRAME_SIZE*(i/2+1);
    
    segment = audio(start_i:end_i, :);
    reference_segment = segment(:, 1);
    
    bands = fmclt_many(segment);
    
    is_speech = vadG729(reference_segment, VAD_cst_param);
%   is_speech = vad(reference_segment, VAD_cst_param);
    if is_speech
        % vad testing
        % speech_only(start_i:end_i, :) = segment;
        
        % add to the speech buffer
        speech_buf(sb_write_i:sb_write_i+MCLT_FRAME-1) = bands(:, 1);
        sb_write_i = sb_write_i + MCLT_FRAME;
        if sb_write_i + MCLT_FRAME - 1 > numel(speech_buf)
            sb_write_i = sb_write_i - numel(speech_buf);
        end
        
        % generate the extra-noisy boi
        little_noise = P .* (noise_buf(nb_read_i:nb_read_i+MCLT_FRAME-1)');
        nb_read_i = nb_read_i + MCLT_FRAME;
        if nb_read_i + MCLT_FRAME - 1 > numel(noise_buf)
            nb_read_i = nb_read_i - numel(noise_buf);
        end
        
        for j=1:size(bands, 2)
            generated_bands(:, j) = bands(:, j) + little_noise; 
        end
    else
        % add to the noise buffer
        noise_buf(nb_write_i:nb_write_i+MCLT_FRAME-1) = bands(:, 1);
        nb_write_i = nb_write_i + MCLT_FRAME;
        if nb_write_i + MCLT_FRAME - 1 > numel(speech_buf)
            nb_write_i = nb_write_i - numel(speech_buf);
        end
        
        % generate the slightly-speechy boi
        little_signal = P .* (speech_buf(sb_read_i:sb_read_i+MCLT_FRAME-1)');
        sb_read_i = sb_read_i + MCLT_FRAME;
        if sb_read_i + MCLT_FRAME - 1 > numel(noise_buf)
            sb_read_i = sb_read_i - numel(noise_buf);
        end
        
        for j=1:size(bands, 2)
            generated_bands(:, j) = bands(:, j) + little_signal; 
        end
    end
    
    factor = 0.5;
    if is_speech
        factor = 1;
    end
    for k=1:size(bands, 2)
        [~, ~, weights] = lmsFilter(complex(generated_bands(1:MCLT_FRAME, k)), complex(generated_bands(1:MCLT_FRAME, 1)));
        out = fimclt(firFilter(complex(bands(1:MCLT_FRAME, k)), weights'));
        filtered_output(start_i:end_i, k) = filtered_output(start_i:end_i, k) + factor * out;
    end
%     toc
end
end

