function tmp = strfilt(list, patterns)
% Filter string list by matching patterns.
% 
% Arguments:
%     list - List to filter
%     patterns - List of regexp patterns
% 
% Return values:
%     tmp - Filtered string list
% 
% Copyright (C) 2008 Johannes Kissel<johannes.kissel@ipe.fzk.de>
% For the licensing terms see ../COPYING.

	if isempty(patterns)
		tmp = list;
		return;
	end

	tmp = {};
	
	for i = 1:length(list)
		x = list{i};
		
		for j = 1:length(patterns)
			p = patterns{j};
			
			if regexpi(x, p)
				tmp = [tmp, x];
				break;
			end
		end
	end
end
