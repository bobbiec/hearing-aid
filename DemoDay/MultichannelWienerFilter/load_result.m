function [input, output, noise, fs] = load_result(name)
    [input, fs] = audioread(sprintf('Recordings/MWF/%s/input.wav', name));
    [output, ~] = audioread(sprintf('Recordings/MWF/%s/output.wav', name));
    [noise, ~] = audioread(sprintf('Recordings/MWF/%s/noise.wav', name));
end

