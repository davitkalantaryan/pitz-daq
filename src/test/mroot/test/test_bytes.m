function test = test_bytes
	
	% create testcase object
    test = munit_testcase;
	
    % create structure of constraints for assertions
    c = munit_constraint;
	
	% test functions
	
	% ip  = input
	% rv  = reference value
	% av  = actual value
	
	% double scalar
	function test_00
		ip = 1;
		
		rv = whos('ip');
		rv = rv.bytes;
		av = bytes(ip);
		
		test.assert(c.eq(rv, av));
	end
	
	% cell array
	function test_01
		ip = {1 'a'};
		
		rv = whos('ip');
		rv = rv.bytes;
		av = bytes(ip);
		
		test.assert(c.eq(rv, av));
	end
	
	% struct scalar
	function test_02
		ip = struct('a', 1, 'b', 2);
		
		rv = whos('ip');
		rv = rv.bytes;
		av = bytes(ip);
		
		test.assert(c.eq(rv, av));
	end

end
