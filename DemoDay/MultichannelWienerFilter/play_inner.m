function play_inner(audio, fs)
    soundsc([audio(:, 2) audio(:, 3)], fs);
end

