function str = implode(array, glue)
% Joins array elements with a glue string.
% 
% Arguments:
%     array - Array of string pieces; default: {''}
%     glue - default: ''
% 
% Return values:
%     str - String representation of all array elements in the same order
%           with glue between them.
% 
% See also: http://de.php.net/implode
% 
% Copyright (C) 2008 Johannes Kissel<johannes.kissel@ipe.fzk.de>
% For the licensing terms see ./COPYING.

	% check args
	if nargin < 2
		glue = '';
		
		if nargin < 1
			array = {''};
		end
	end
	
	% compose str
	str = '';
	
	if ~isempty(array)
		for x = array(1:end-1)
			str = [str char(x) glue];
		end
		
		str = [str char(array(end))];
	end
end
