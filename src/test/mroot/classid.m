function id = classid(var)
% Get a variables class as numerical value.
% 
% Arguments:
%     var - The variable
% 
% Return values:
%     id - The mxClassID corresponding to the class
% 
% See also:
%     http://www.mathworks.com/access/helpdesk_r13/help/techdoc/apiref/mxclassid.html
% 
% Copyright (C) 2008 Johannes Kissel<johannes.kissel@ipe.fzk.de>
% For the licensing terms see ./COPYING.

	if     iscell(var);        id =  1;
	elseif isstruct(var);      id =  2;
	elseif islogical(var);     id =  3;
	elseif ischar(var);        id =  4;
	elseif isa(var, 'double'); id =  6;
	elseif isa(var, 'single'); id =  7;
	elseif isa(var,  'int8');  id =  8;
	elseif isa(var, 'uint8');  id =  9;
	elseif isa(var,  'int16'); id = 10;
	elseif isa(var, 'uint16'); id = 11;
	elseif isa(var,  'int32'); id = 12;
	elseif isa(var, 'uint32'); id = 13;
	elseif isa(var,  'int64'); id = 14;
	elseif isa(var, 'uint64'); id = 15;
	else                       id =  5;
	end
end
