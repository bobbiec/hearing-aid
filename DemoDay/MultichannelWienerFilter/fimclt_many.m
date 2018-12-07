function X_many = fimclt_many(x)
    X_many = zeros(size(x, 1)*2, size(x,2));
    columns = size(x, 2);
    for i=1:columns
        X_many(:, i) = fimclt(x(:, i));
    end
    return;