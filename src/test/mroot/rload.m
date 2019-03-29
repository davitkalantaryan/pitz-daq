function varargout = rload(filename, varargin)
% Loads workspace variables from a ROOT File.
% 
% Arguments:
%     filename - Path to ROOT-File (see also pathinfo.m); if no extension,
%                '.root' is appended; default: 'matlab.root' in pwd
%     varargin - optional; e.g. variable patterns
%       *  -regexp: variables matching regexp patterns
%       * patterns: wildcards allowed
% 
% Return values:
%     varargout - optional; struct with loaded vars as fields
%
% Synopsis:
%     [s =] rload [filename [-regexp] [patterns]]
% 
%     e.g. rload matlab.root -regexp a.*
% 
% See also:
%     http://www.mathworks.com/access/helpdesk/help/techdoc/ref/load.html
% 
% Copyright (C) 2008 Johannes Kissel<johannes.kissel@ipe.fzk.de>
% For the licensing terms see ./COPYING.

	% check for non-string arguments
	if ~all(cellfun(@(x) ischar(x), varargin))
		error('Argument must contain a string.');
	end
	
	% no arguments given
	if nargin < 1
		filename = 'matlab.root'; % default filename
	end
	
	% parse filename
	[ofile, ifile] = pathinfo(filename);
	
	if ofile.type > -1 % ROOT file
		% build variable list
		e = ['rwho(''-file'', ''' filename];
		if ~isempty(varargin); e = [e ''', ''' implode(varargin, ''', ''')]; end
		e = [e ''')'];
		vlist = eval(e);
		
		% prepare ofile
		ofile.version = rversion('-compatible');
		
		% load variables
		s = mex_rload({ofile ifile vlist});
	else % other cases
		s = evalin('caller', ['load ' filename ' ' implode(varargin, ' ')]);
	end
	
	% output
	if nargout > 0 % as struct
		varargout{1} = s;
	else % as variables
		names = fieldnames(s);
		
		for i = 1:length(names)
			name = names{i};
			assignin('caller', name, s.(name));
		end
	end
end
