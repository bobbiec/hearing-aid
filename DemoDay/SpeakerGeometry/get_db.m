function db = get_db(input)
% https://dsp.stackexchange.com/a/4892
    db = 10*log10(var(input));
end

