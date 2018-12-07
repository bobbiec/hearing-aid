
function X_many = fmclt_many(x)
    X_many = complex(zeros(size(x, 1)/2, size(x,2)), 0);
    columns = size(x, 2);
    for i=1:columns
        X_many(:, i) = fmclt(x(:, i));
    end
    return;

 