function varargout = rwhos(varargin)
% Show info about variables in workspace or file.
% 
% Arguments:
%     varargin - optional; e.g. filename and variable patterns
%       * -file <filename>: Variables in file
%       * -regexp: Variables matching regexp patterns
%       * <patterns>: Wildcards allowed
% 
% Return values:
%     varargout - optional; array with variable info
%                 (displayed if not returned)
% 
% Synopsis:
%     [<struct> =] rwhos [[-file <filename>] [-regexp] [<patterns>]]
% 
%     e.g. s = rwhos -file matlab.root a*
% 
% See also:
%     http://www.mathworks.com/access/helpdesk/help/techdoc/ref/whos.html
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
			
			% load variable list
			vlist = eval(['rwho(''' implode(varargin, ''', '''), ''')']);
			
			% generate vinfo
			vinfo = struct([]);
			
			% loop over variables
			for i = 1:length(vlist)
				x = vlist{i};
				
				% load variable
				v = mex_rload({ofile ifile {x}});
				v = v.(x);
				
				% buffer vinfo
				vinfo = [vinfo; whos('v')];
				vinfo(i).name = x;
			end
		end
	end
	
	% args are not ROOT related; delegate to built-in who function
	if ~root
		vinfo = evalin('caller', ['whos(''' implode(varargin, ''', ''') ''')']);
	end
	
	% output
	if nargout > 0         % return
		varargout = {vinfo};
	elseif ~isempty(vinfo) % display
		% print table header
		disp(sprintf('\n  %-12s  %-12s  %10s  %-6s  %s  ', ...
			'Name',      ...
			'Size',      ...
			'Bytes',     ...
			'Class',     ...
			'Attributes' ...
		));
		
		% print table rows
		for i = 1:length(vinfo)
			v = vinfo(i);
			
			% adjust columns
			
			% name
			if length(v.name) > 12
				n = [v.name(1:9) '...'];
			else
				n =  v.name;
			end
			
			% size
			if length(v.size) > 3
				s = sprintf('%id', length(v.size));
			else
				s = implode(arrayfun(@(i) num2str(i), v.size, 'UniformOutput', 0), 'x');
			end
			
			% attributes
			a = {};
			
			if v.global;     a = [a 'global'];     end
			if v.sparse;     a = [a 'sparse'];     end
			if v.complex;    a = [a 'complex'];    end
			if v.persistent; a = [a 'persistent']; end
			
			a = implode(a, ', ');
			
			% print
			disp(sprintf('  %-12s  %-12s  %10i  %-6s  %s  ', n, s, v.bytes, v.class, a));
		end
		
		% print empty row
		disp(' ');
	end
end
