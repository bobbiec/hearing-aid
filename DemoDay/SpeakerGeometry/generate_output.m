function y = generate_output(speaker_audio, sample_differences, speaker_number)
% Inputs:
% speaker_audio: N x 1 representing audio played from speaker
% sample_differences: channels x speakers matrix of sample delays between mic and speaker
% speaker_number: which speaker is being used
%
% Outputs:
% y: ~N x channels audio delayed and attenuated as if received by mics from the source speaker_number
%    (zero-padded to be rectangular)
%
% Note: four-channel audio doesn't usually play nicely by default.
% For Teensy use you'll want to split into quad(:, 1:2) and quad(:, 3:4)

    diffs = sample_differences(:, speaker_number);
    
    % Use min_diff to compute sound attenuation - amplitudes decrease by 1/r^2
    % Set min_diff as 1 and scale appropriately
    min_diff_sq = min(diffs) ^ 2;
    
    % Use max_diff to compute proper padding so the matrix fits
    max_diff = max(diffs);
    
    y = zeros(size(speaker_audio, 1) + max_diff, size(sample_differences, 1));
    
    for i=1:length(diffs)
        attenuated_input = min_diff_sq / (diffs(i)^2) .* speaker_audio;
        new = vertcat(zeros(diffs(i), 1), attenuated_input);
        y(:, i) = padarray(new, max_diff-diffs(i), 'post');
    end
end