function rsave(filename, varargin)
% Saves workspace variables to a ROOT File.
% 
% Arguments:
%     filename - Path to ROOT-File (see also pathinfo.m); if no extension,
%                '.root' is appended; default: 'matlab.root' in pwd
%     varargin - optional; e.g. variable patterns
%       *  -struct: fields of scalar struct variable
%       *  -regexp: variables matching regexp patterns
%       * patterns: wildcards allowed
%       *  -append: append to inner directory
%
% Synopsis:
%     rsave [filename [-struct s] [-regexp] [patterns] [-append]]
% 
%     e.g. rsave matlab.root -regexp a.* -append
% 
% See also:
%     http://www.mathworks.com/access/helpdesk/help/techdoc/ref/save.html
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
	
	% delegate to built-in save function
	if ofile.type < 0 % no ROOT file
		evalin('caller', ['save ' filename ' ' implode(varargin, ' ')]);
		return;
	end
	
	% only filename given
	if nargin < 2
		vlist  = evalin('caller', 'who'''); % all variables in caller workspace
		names  = vlist;
		
		version = '1.0';
		append  = 0;
	else
		% check for option flags
		
		% struct flag
		struct = 0;
		
		if strcmpi(varargin{1}, '-struct')
			if length(varargin) < 2 ...
					|| evalin('caller', ['~exist(''' varargin{2} ''', ''var'')']) ...
					|| evalin('caller', ['~isstruct(' varargin{2} ')'])
				error('The -STRUCT option must be followed by the name of a scalar structure variable.');
			end
			
			struct   = varargin{2};
			varargin = {varargin{3:end}};
		end
		
		% regexp flag
		regexp = 0;
		
		if ~isempty(varargin) && strcmpi(varargin{1}, '-regexp')
			regexp = 1;
			varargin = {varargin{2:end}};
		end
		
		% version flag
		version = rversion('-compatible');
		
		if ~isempty(varargin) && length(varargin{end}) > 2 && strcmpi(varargin{end}(1:2), '-v')
			ver = varargin{end}(3:end);
			v   = vercmp(version, ver);
			
			if ~isnan(v) && v >= 0 && vercmp('1.0', varargin{end}) <= 0 % 1.0: first mroot version
				version  = ver;
				varargin = {varargin{1:end}};
			end
		end
		
		% append flag
		append = 0;
		
		if ~isempty(varargin) && strcmpi(varargin{end}, '-append')
			append   = 1;
			varargin = {varargin{1:(end - 1)}};
		end
		
		% check for non-string arguments and unknown command options
		for i = 1:length(varargin)
			x = varargin{i};
			
			if ~isempty(x) && x(1) == '-'
				error('Unknown command option ''%s''.', x);
			end
		end
		
		% build variable list
		vlist = {};
		names = {};
		
		if struct % struct fields
			all_flds = evalin('caller', ['fields(' struct ')''']);
			
			if isempty(varargin)
				names = all_flds;
				vlist = cellfun(@(x) [struct '.' x], all_flds, 'UniformOutput', 0);
			else
				for i = 1:length(varargin)
					x = varargin{i};
					a = apply(x, regexp, all_flds);
					
					if isempty(a)
						if regexp || ~isempty(strfind(x, '*'))
							error('The variable ''%s'' does not contain a field matching the pattern ''%s''.', struct, x);
						else
							error('The variable ''%s'' does not contain a field named ''%s''.', struct, x);
						end
					else
						for j = a
							names = addstr(names, all_flds{j});
							vlist = addstr(vlist, [struct '.' all_flds{j}]);
						end
					end
				end
			end
		else % variables
			all_vars = evalin('caller', 'who''');
			
			if isempty(varargin)
				vlist = all_vars;
			else
				for i = 1:length(varargin)
					x = varargin{i};
					a = apply(x, regexp, all_vars);
					
					if isempty(a)
						if regexp || ~isempty(strfind(x, '*'))
							error('No variable matched the pattern ''%s''.', x);
						else
							error('Variable ''%s'' not found.', x);
						end
					else
						for j = a
							vlist = addstr(vlist, all_vars{j});
						end
					end
				end
			end
			
			names = vlist;
		end
	end
	
	% prepare o- and i-file
	ofile.version = version;
	ifile.append  = append;
	
	% export info for mex call to caller workspace
	assignin('caller', 'rootinfo', {ofile ifile names});
	
	% call mex file in caller context
	evalin('caller', ['mex_rsave(rootinfo, ' implode(vlist, ', ') ')']);
	
	% clean up caller workspace
	evalin('caller', 'clear rootinfo');
end

function a = apply(pattern, regexp, all)
% Determine whether pattern matches variables in 'all'.
% 
% Arguments:
%     pattern - The pattern
%     regexp - 1 if pattern is a regexp
%     all - list of variable names
% 
% Return values:
%     a - The indexes of variables which are matched by pattern
% 
% Copyright (C) 2008 Johannes Kissel<johannes.kissel@ipe.fzk.de>
% For the licensing terms see ./COPYING.

	a = [];
	
	if ~regexp
		if strfind(pattern, '*') % wildcards are handled as regexp
			pattern = strrep(pattern, '*', '.*');
			regexp  = 1;
		else
			a = find(cellfun(@(x) strcmp(x, pattern), all), 1);
		end
	end
	
	if  regexp
		for i = 1:length(all)
			if regexpi(all{i}, pattern)
				a = [a i];
			end
		end
	end
end

function array = addstr(array, str)
% Adds a string to a cell array.
% 
% Arguments:
%     array - The array
%     str - The string
% 
% Return values:
%     array - The expanded array
% 
% Copyright (C) 2008 Johannes Kissel<johannes.kissel@ipe.fzk.de>
% For the licensing terms see ./COPYING.

	if isempty(find(cellfun(@(x) strcmp(x, str), array), 1))
		array = [array str];
	end
end
