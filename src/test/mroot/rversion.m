function out = rversion(varargin)
% Returns mroot version information.
% 
% Arguments:
%     varargin - Options
%       *  -compatible: Return minimum compatible release
%       *        -date: Return release date
%       * -description: Return release description
%       *        -java: Return java version information
% 
% Return values:
%     out - Output depending on options;
%           current version number if no options
% 
% See also:
%     http://www.mathworks.com/access/helpdesk/help/techdoc/ref/version.html
% 
% Copyright (C) 2008 Johannes Kissel<johannes.kissel@ipe.fzk.de>
% For the licensing terms see ./COPYING

	if nargin > 1
		error('Too many input arguments.');
	end

	if nargin == 0
		out = '1.0';
	elseif strcmpi(varargin, '-compatible')
		out = '1';
	elseif strcmpi(varargin, '-date')
		out = 'August 11, 2008';
	elseif strcmpi(varargin, '-description')
		out = 'Initial release.';
	elseif strcmpi(varargin, '-java')
		out = version('-java');
	elseif strcmpi(varargin, '-release')
		out = ''; % not implemented
	else
		error('Invalid option passed to ''version'' command.');
	end
end
