INPUT_NAME = 'picture_white-16';

mkdir(sprintf('Recordings/MWF/%s', INPUT_NAME));
FRAME_SIZE = 160; % required to use vadG729
P = 1; % the p from the Microsoft paper

[audio, fs] = audioread('Recordings/Geometry/result.wav');
audiowrite(sprintf('Recordings/MWF/%s/input.wav', INPUT_NAME), audio, fs);

% pad out to frame size
padding_length = FRAME_SIZE - mod(size(audio, 1), FRAME_SIZE);
audio = [audio; zeros(padding_length, size(audio, 2))];

% system object stuff
VAD_cst_param = vadInitCstParams;
VAD_cst_param.Fs = 48000;
VAD_cst_param.L_WINDOW = FRAME_SIZE*3;
VAD_cst_param.L_NEXT = FRAME_SIZE/2;
VAD_cst_param.L_FRAME = FRAME_SIZE;
VAD_cst_param.hamwindow = hamming(FRAME_SIZE*3);
clear vadG729

filtered_output = mwf(audio, FRAME_SIZE, P, VAD_cst_param);
audiowrite(sprintf('Recordings/MWF/%s/output.wav', INPUT_NAME), filtered_output, fs);

noise = filtered_output - padarray(quad_main, size(filtered_output, 1)-size(quad_main, 1), 'post');
audiowrite(sprintf('Recordings/MWF/%s/noise.wav', INPUT_NAME), noise, fs);

db_diff = get_db(audio) - get_db(filtered_output);
fprintf('Removed %0.2f-%0.2f dB of (hopefully) noise\n', min(db_diff), max(db_diff));