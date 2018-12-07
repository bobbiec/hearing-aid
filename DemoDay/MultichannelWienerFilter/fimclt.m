function y = fimclt(X)  
  
% FIMCLT - Compute IMCLT of a vector via double-length FFT  
%  
% H. Malvar, September 2001  --  (c) 1998-2001 Microsoft Corp.  
%  
% Syntax:  y = fimclt(X)  
%  
% Input:   X : complex-valued MCLT coefficients, M subbands  
%  
% Output:  y : real-valued output vector of length 2*M  
  
% in Matlab, by default j = sqrt(-1)  
  
% determine # of subbands, M  
M = length(X);  
  
% allocate vector Y   
Y = complex(zeros(2*M,1));  
  
% compute modulation function  
k = [1:M-1]';  
c = W(8,2*k+1) .* W(4*M,k);  
  
% map X into Y  
new_y = (1/4) * conj(c) .* (X(1:M-1) - j * X(2:M));
Y(2:M) = (1/4) * conj(c) .* (X(1:M-1) - j * X(2:M));  
  
% determine first and last Y values  
Y(1)   =   sqrt(1/8) * (real(X(1)) + imag(X(1)));  
Y(M+1) = - sqrt(1/8) * (real(X(M)) + imag(X(M)));  
  
% complete vector Y via conjugate symmetry property for the  
% FFT of a real vector (not needed if the inverse FFT  
% routine is a "real FFT", which should take only as input  
% only M+1 coefficients)  
Y(M+2:2*M) = conj(Y(M:-1:2));  
  
% inverse normalized FFT to compute the output vector  
% output of ifft should have zero imaginary part; but  
% by calling real(.) we remove the small rounding noise  
% that's present in the imaginary part  
y = real(ifft(sqrt(2*M) * Y));  
  
return; 
 
function w = W(M,r)  
  
w = exp(-j*2*pi*r/M);  
  
return;  