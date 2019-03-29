function array = explode(str, delimiter)
% Splits a string by string
% 
% Arguments:
%     str - The input string; default: ''
%     delimiter - The boundary string; default: ' '
% 
% Return values:
%     array - An array of strings, each is a substring of 'str' limited
%             by the 'delimiter'.
% 
% See also: http://de.php.net/explode
% 
% Copyright (C) 2008 Johannes Kissel<johannes.kissel@ipe.fzk.de>
% For the licensing terms see ./COPYING.

	array = {};
	
	% check args
	if nargin < 2
		delimiter = ' ';
		
		if nargin < 1
			str = '';
		end
	end
	
	% split str and fill array
	while true
		[tok, str] = strtok(str, delimiter);
		array = [array tok];
		
		if isempty(tok), break; end
	end
	
	array = [array str(2:end)];
end
