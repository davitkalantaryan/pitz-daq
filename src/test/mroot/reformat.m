function ovar = reformat(ivar, direction)
% Reformat a variable for easier transformation.
%
% Arguments:
%     ivar - The input variable
%     direction - Direction of transformation; MX2ROOT = 1, ROOT2MX = 2
%
% Return values:
%     ovar - The reformated output variable
%
% Copyright (C) 2008 Johannes Kissel<johannes.kissel@ipe.fzk.de>
% For the licensing terms see ./COPYING.

	if direction == 1 % MX2ROOT
		ovar.type = classid(ivar);
		
		if isstruct(ivar)
			ovar.fields = fieldnames(ivar);
			ivar = struct2cell(ivar);
		end
		
		ovar.size = size(ivar);
		ovar.data = reshape(ivar, 1, []);
		
		if isnumeric(ivar)
			if isreal(ivar)
				ovar.complex = 0;
			else
				ovar.real    = real(ovar.data);
				ovar.imag    = imag(ovar.data);
				ovar.complex = 1;
				
				ovar = rmfield(ovar, 'data');
			end
		end
	else % ROOT2MX
		if ivar.type > 5 && ivar.complex == 1 % complex numbers
			ivar.data = complex(ivar.real, ivar.imag);
		end
		
		ovar = reshape(ivar.data, ivar.size);
		
		if ivar.type == 2 % struct array
			ovar = cell2struct(ovar, ivar.fields, 1);
		end
	end
end
