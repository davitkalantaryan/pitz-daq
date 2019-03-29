function varargout = rwho(varargin)
% Lists variables in workspace or file.
% 
% Arguments:
%     varargin - optional; e.g. filename and variable patterns
%       * -file <filename>: Variables in file
%       * -regexp: Variables matching regexp patterns
%       * <patterns>: Wildcards allowed
% 
% Return values:
%     varargout - optional; list of variables (displayed if not returned)
% 
% Synopsis:
%     [<struct> =] rwho [[-file <filename>] [-regexp] [<patterns>]]
% 
%     e.g. s = rwho -file matlab.root a*
% 
% See also:
%     http://www.mathworks.com/access/helpdesk/help/techdoc/ref/who.html
% 
% Copyright (C) 2008 Johannes Kissel<johannes.kissel@ipe.fzk.de>
% For the licensing terms see ./COPYING.

	% check for non-string arguments
	if ~all(cellfun(@(x) ischar(x), varargin))
		error('Argument must contain a string.');
	end
	
	root = 0; % default: args are not ROOT related
	
	if nargin >= 2 && strcmpi(varargin{1}, '-file')
		[ofile, ifile] = pathinfo(varargin{2});
		
		% args are ROOT related
		if ofile.type > -1
			root = 1;
			varargin = {varargin{3:end}};
			
			% check for regexp flag
			regexp = 0;
			
			if ~isempty(varargin) && strcmp(varargin{1}, '-regexp')
				regexp = 1;
				varargin = {varargin{2:end}};
			end
			
			% check for unknown command options
			if ~all(cellfun(@(x) x(1) ~= '-', varargin))
				error('Unknown command option.');
			end
			
			% transform patterns to regexps
			if ~regexp
				varargin = cellfun(@(x) ['^' strrep(x, '*', '.*') '$'], varargin, 'UniformOutput', 0);
			end
			
			% prepare ofile
			ofile.version = rversion('-compatible');
			
			% load and filter variable list
			vlist = mex_rwho({ofile, ifile});
			vlist = strfilt(vlist, varargin);
		end
	end
	
	% args are not ROOT related; delegate to built-in who function
	if ~root
		vlist = evalin('caller', ['who(''' implode(varargin, ''', '), ''')']);
	end
	
	% output
	if nargout > 0         % return
		varargout{1} = vlist;
	elseif ~isempty(vlist) % display
		disp(sprintf('\nYour variables are:\n'));
		cellfun(@(v) disp(sprintf('\t%s', v)), vlist);
		disp(' ');
	end
end
