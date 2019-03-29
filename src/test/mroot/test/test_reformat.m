function test = test_reformat
	
	% create testcase object
    test = munit_testcase;
	
    % create structure of constraints for assertions
    c = munit_constraint;
	
	% test functions
	
	% ip  = input
	% rv  = reference value
	% av  = actual value
	% dir = direction of transformation; 1 = mx2root, 2 = root2mx
	
	% root2mx
	
	% cell array
	function test_00
		ip  = struct( ...
			'type',   1, ...
			'size',  [1 2], ...
			'data', {{1 'a'}} ...
		);
		dir = 0;
		
		rv = {1 'a'};
		av = reformat(ip, dir);
		
		test.assert(c.same(rv, av));
	end
	
	% struct array
	function test_01
		ip  = struct( ...
			'type',     2, ...
			'fields', {{'a' 'b'}'}, ...
			'size',    [2 1], ...
			'data',   {{1 2}} ...
		);
		dir = 0;
		
		rv = struct('a', 1, 'b', 2);
		av = reformat(ip, dir);
		
		test.assert(c.same(rv, av));
	end
	
	% logical matrix
	function test_02
        ip  = struct( ...
			'type',  3, ...
			'size', [3 3], ...
			'data', logical([1 0 0 0 1 0 0 0 1]) ...
		);
		dir = 0;
		
		rv = logical(eye(3));
		av = reformat(ip, dir);
		
		test.assert(c.eq(rv, av));
	end
	
	% char array
	function test_03
        ip  = struct( ...
			'type',  4, ...
			'size', [1 3], ...
			'data', 'abc' ...
		);
		dir = 0;
		
		rv = 'abc';
		av = reformat(ip, dir);
		
		test.assert(c.eq(rv, av));
	end
	
	% double matrix
	function test_04
        ip  = struct( ...
			'type',     6, ...
			'size',    [3 3], ...
			'data',    [1 0 0 0 1 0 0 0 1], ...
			'complex',  0 ...
		);
		dir = 0;
		
		rv = eye(3);
		av = reformat(ip, dir);
		
		test.assert(c.eq(rv, av));
	end
	
	% complex double matrix
	function test_05
        ip  = struct( ...
			'type',     6, ...
			'size',    [3 3], ...
			'real',    [1 0 0 0 1 0 0 0 1], ...
			'imag',    [1 0 0 0 1 0 0 0 1], ...
			'complex',  1 ...
		);
		dir = 0;
		
		rv = complex(eye(3), eye(3));
		av = reformat(ip, dir);
		
		test.assert(c.eq(rv, av));
	end
	
	% int matrix
	function test_06
        ip = struct( ...
			'type',     8, ...
			'size',    [3 3], ...
			'data',    int8([1 0 0 0 1 0 0 0 1]), ...
			'complex',  0 ...
		);
		dir = 0;
		
		rv = int8(eye(3));
		av = reformat(ip, dir);
		
		test.assert(c.eq(rv, av));
	end

	% cell array with nested struct and cell array
	function test_07
		ip = struct( ...
			'type',   1, ...
			'size',  [1 4], ...
			'data', {{1 'a' {1 'a'} struct('a', 1, 'b', 2)}} ...
		);
		dir = 0;
		
		rv = {1 'a' {1 'a'} struct('a', 1, 'b', 2)};
		av = reformat(ip, dir);
		
		test.assert(c.same(rv, av));
	end
	
	% mx2root
	
	% cell array
	function test_10
		ip  = {1 'a'};
		dir = 1;
		
		rv = struct( ...
			'type',   1, ...
			'size',  [1 2], ...
			'data', {{1 'a'}} ...
		);
		av = reformat(ip, dir);
		
		test.assert(c.same(rv, av));
	end
	
	% struct array
	function test_11
		ip  = struct('a', 1, 'b', 2);
		dir = 1;
		
		rv = struct( ...
			'type',     2, ...
			'fields', {{'a' 'b'}'}, ...
			'size',    [2 1], ...
			'data',   {{1 2}} ...
		);
		av = reformat(ip, dir);
		
		test.assert(c.same(rv, av));
	end
	
	% logical matrix
	function test_12
        ip  = logical(eye(3));
		dir = 1;
		
		rv = struct( ...
			'type',  3, ...
			'size', [3 3], ...
			'data', logical([1 0 0 0 1 0 0 0 1]) ...
		);
		av = reformat(ip, dir);
		
		test.assert(c.isa(av.data, 'logical'));
		
		test.assert(c.eq(rv.type, av.type));
		test.assert(c.eq(rv.size, av.size));
		test.assert(c.eq(rv.data, av.data));
		
		test.assert(c.same(fields(rv), fields(av)));
	end
	
	% char array
	function test_13
        ip  = 'abc';
		dir = 1;
		
		rv = struct( ...
			'type',  4, ...
			'size', [1 3], ...
			'data', 'abc' ...
		);
		av = reformat(ip, dir);
		
		test.assert(c.isa(av.data, 'char'));
		test.assert(c.same(rv, av));
	end
	
	% double matrix
	function test_14
        ip  = eye(3);
		dir = 1;
		
		rv = struct( ...
			'type',     6, ...
			'size',    [3 3], ...
			'data',    [1 0 0 0 1 0 0 0 1], ...
			'complex',  0 ...
		);
		av = reformat(ip, dir);
		
		test.assert(c.isa(av.data, 'double'));
		test.assert(c.same(rv, av));
	end
	
	% complex double matrix
	function test_15
        ip  = complex(eye(3), eye(3));
		dir = 1;
		
		rv = struct( ...
			'type',     6, ...
			'size',    [3 3], ...
			'real',    [1 0 0 0 1 0 0 0 1], ...
			'imag',    [1 0 0 0 1 0 0 0 1], ...
			'complex',  1 ...
		);
		av = reformat(ip, dir);
		
		test.assert(c.isa(av.real, 'double'));
		test.assert(c.isa(av.imag, 'double'));
		
		test.assert(c.same(rv, av));
	end
	
	% int matrix
	function test_16
        ip  = int8(eye(3));
		dir = 1;
		
		rv = struct( ...
			'type',     8, ...
			'size',    [3 3], ...
			'data',    int8([1 0 0 0 1 0 0 0 1]), ...
			'complex',  0 ...
		);
		av = reformat(ip, dir);
		
		test.assert(c.isa(av.data, 'int8'));
		test.assert(c.same(rv, av));
	end

	% cell array with nested struct and cell array
	function test_17
		ip  = {1 'a' {1 'a'} struct('a', 1, 'b', 2)};
		dir = 1;
		
		rv = struct( ...
			'type',  1, ...
			'size', [1 4], ...
			'data', {{1 'a' {1 'a'} struct('a', 1, 'b', 2)}} ...
		);
		av = reformat(ip, dir);
		
		test.assert(c.same(rv, av));
	end
	
end
