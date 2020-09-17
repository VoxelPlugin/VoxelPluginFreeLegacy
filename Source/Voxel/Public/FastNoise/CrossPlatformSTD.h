// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include <istream>
#include <limits>

// While random generators are standardized, samplers aren't
// To avoid issues, we use the microsoft std version on all platforms
// License:

/*
 The Microsoft C++ Standard Library is under the Apache License v2.0 with LLVM Exception:

                                 Apache License
                           Version 2.0, January 2004
                        http://www.apache.org/licenses/

   TERMS AND CONDITIONS FOR USE, REPRODUCTION, AND DISTRIBUTION

   1. Definitions.

      "License" shall mean the terms and conditions for use, reproduction,
      and distribution as defined by Sections 1 through 9 of this document.

      "Licensor" shall mean the copyright owner or entity authorized by
      the copyright owner that is granting the License.

      "Legal Entity" shall mean the union of the acting entity and all
      other entities that control, are controlled by, or are under common
      control with that entity. For the purposes of this definition,
      "control" means (i) the power, direct or indirect, to cause the
      direction or management of such entity, whether by contract or
      otherwise, or (ii) ownership of fifty percent (50%) or more of the
      outstanding shares, or (iii) beneficial ownership of such entity.

      "You" (or "Your") shall mean an individual or Legal Entity
      exercising permissions granted by this License.

      "Source" form shall mean the preferred form for making modifications,
      including but not limited to software source code, documentation
      source, and configuration files.

      "Object" form shall mean any form resulting from mechanical
      transformation or translation of a Source form, including but
      not limited to compiled object code, generated documentation,
      and conversions to other media types.

      "Work" shall mean the work of authorship, whether in Source or
      Object form, made available under the License, as indicated by a
      copyright notice that is included in or attached to the work
      (an example is provided in the Appendix below).

      "Derivative Works" shall mean any work, whether in Source or Object
      form, that is based on (or derived from) the Work and for which the
      editorial revisions, annotations, elaborations, or other modifications
      represent, as a whole, an original work of authorship. For the purposes
      of this License, Derivative Works shall not include works that remain
      separable from, or merely link (or bind by name) to the interfaces of,
      the Work and Derivative Works thereof.

      "Contribution" shall mean any work of authorship, including
      the original version of the Work and any modifications or additions
      to that Work or Derivative Works thereof, that is intentionally
      submitted to Licensor for inclusion in the Work by the copyright owner
      or by an individual or Legal Entity authorized to submit on behalf of
      the copyright owner. For the purposes of this definition, "submitted"
      means any form of electronic, verbal, or written communication sent
      to the Licensor or its representatives, including but not limited to
      communication on electronic mailing lists, source code control systems,
      and issue tracking systems that are managed by, or on behalf of, the
      Licensor for the purpose of discussing and improving the Work, but
      excluding communication that is conspicuously marked or otherwise
      designated in writing by the copyright owner as "Not a Contribution."

      "Contributor" shall mean Licensor and any individual or Legal Entity
      on behalf of whom a Contribution has been received by Licensor and
      subsequently incorporated within the Work.

   2. Grant of Copyright License. Subject to the terms and conditions of
      this License, each Contributor hereby grants to You a perpetual,
      worldwide, non-exclusive, no-charge, royalty-free, irrevocable
      copyright license to reproduce, prepare Derivative Works of,
      publicly display, publicly perform, sublicense, and distribute the
      Work and such Derivative Works in Source or Object form.

   3. Grant of Patent License. Subject to the terms and conditions of
      this License, each Contributor hereby grants to You a perpetual,
      worldwide, non-exclusive, no-charge, royalty-free, irrevocable
      (except as stated in this section) patent license to make, have made,
      use, offer to sell, sell, import, and otherwise transfer the Work,
      where such license applies only to those patent claims licensable
      by such Contributor that are necessarily infringed by their
      Contribution(s) alone or by combination of their Contribution(s)
      with the Work to which such Contribution(s) was submitted. If You
      institute patent litigation against any entity (including a
      cross-claim or counterclaim in a lawsuit) alleging that the Work
      or a Contribution incorporated within the Work constitutes direct
      or contributory patent infringement, then any patent licenses
      granted to You under this License for that Work shall terminate
      as of the date such litigation is filed.

   4. Redistribution. You may reproduce and distribute copies of the
      Work or Derivative Works thereof in any medium, with or without
      modifications, and in Source or Object form, provided that You
      meet the following conditions:

      (a) You must give any other recipients of the Work or
          Derivative Works a copy of this License; and

      (b) You must cause any modified files to carry prominent notices
          stating that You changed the files; and

      (c) You must retain, in the Source form of any Derivative Works
          that You distribute, all copyright, patent, trademark, and
          attribution notices from the Source form of the Work,
          excluding those notices that do not pertain to any part of
          the Derivative Works; and

      (d) If the Work includes a "NOTICE" text file as part of its
          distribution, then any Derivative Works that You distribute must
          include a readable copy of the attribution notices contained
          within such NOTICE file, excluding those notices that do not
          pertain to any part of the Derivative Works, in at least one
          of the following places: within a NOTICE text file distributed
          as part of the Derivative Works; within the Source form or
          documentation, if provided along with the Derivative Works; or,
          within a display generated by the Derivative Works, if and
          wherever such third-party notices normally appear. The contents
          of the NOTICE file are for informational purposes only and
          do not modify the License. You may add Your own attribution
          notices within Derivative Works that You distribute, alongside
          or as an addendum to the NOTICE text from the Work, provided
          that such additional attribution notices cannot be construed
          as modifying the License.

      You may add Your own copyright statement to Your modifications and
      may provide additional or different license terms and conditions
      for use, reproduction, or distribution of Your modifications, or
      for any such Derivative Works as a whole, provided Your use,
      reproduction, and distribution of the Work otherwise complies with
      the conditions stated in this License.

   5. Submission of Contributions. Unless You explicitly state otherwise,
      any Contribution intentionally submitted for inclusion in the Work
      by You to the Licensor shall be under the terms and conditions of
      this License, without any additional terms or conditions.
      Notwithstanding the above, nothing herein shall supersede or modify
      the terms of any separate license agreement you may have executed
      with Licensor regarding such Contributions.

   6. Trademarks. This License does not grant permission to use the trade
      names, trademarks, service marks, or product names of the Licensor,
      except as required for reasonable and customary use in describing the
      origin of the Work and reproducing the content of the NOTICE file.

   7. Disclaimer of Warranty. Unless required by applicable law or
      agreed to in writing, Licensor provides the Work (and each
      Contributor provides its Contributions) on an "AS IS" BASIS,
      WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
      implied, including, without limitation, any warranties or conditions
      of TITLE, NON-INFRINGEMENT, MERCHANTABILITY, or FITNESS FOR A
      PARTICULAR PURPOSE. You are solely responsible for determining the
      appropriateness of using or redistributing the Work and assume any
      risks associated with Your exercise of permissions under this License.

   8. Limitation of Liability. In no event and under no legal theory,
      whether in tort (including negligence), contract, or otherwise,
      unless required by applicable law (such as deliberate and grossly
      negligent acts) or agreed to in writing, shall any Contributor be
      liable to You for damages, including any direct, indirect, special,
      incidental, or consequential damages of any character arising as a
      result of this License or out of the use or inability to use the
      Work (including but not limited to damages for loss of goodwill,
      work stoppage, computer failure or malfunction, or any and all
      other commercial damages or losses), even if such Contributor
      has been advised of the possibility of such damages.

   9. Accepting Warranty or Additional Liability. While redistributing
      the Work or Derivative Works thereof, You may choose to offer,
      and charge a fee for, acceptance of support, warranty, indemnity,
      or other liability obligations and/or rights consistent with this
      License. However, in accepting such obligations, You may act only
      on Your own behalf and on Your sole responsibility, not on behalf
      of any other Contributor, and only if You agree to indemnify,
      defend, and hold each Contributor harmless for any liability
      incurred by, or claims asserted against, such Contributor by reason
      of your accepting any such warranty or additional liability.

   END OF TERMS AND CONDITIONS

   APPENDIX: How to apply the Apache License to your work.

      To apply the Apache License to your work, attach the following
      boilerplate notice, with the fields enclosed by brackets "[]"
      replaced with your own identifying information. (Don't include
      the brackets!)  The text should be enclosed in the appropriate
      comment syntax for the file format. We also recommend that a
      file or class name and description of purpose be included on the
      same "printed page" as the copyright notice for easier
      identification within third-party archives.

   Copyright [yyyy] [name of copyright owner]

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

---- LLVM Exceptions to the Apache 2.0 License ----

As an exception, if, as a result of your compiling your source code, portions
of this Software are embedded into an Object form of such source code, you
may redistribute such embedded portions in such Object form without complying
with the conditions of Sections 4(a), 4(b) and 4(d) of the License.

In addition, if you combine or link compiled forms of this Software with
software that is licensed under the GPLv2 ("Combined Software") and if a
court of competent jurisdiction determines that the patent provision (Section
3), the indemnity provision (Section 9) or other Section of the License
conflicts with the conditions of the GPLv2, you may retroactively and
prospectively choose to deem waived or otherwise exclude such Section(s) of
the License, but only in their entirety and only with respect to the Combined
Software.
*/

namespace cross_platform_std
{
	// CLASS TEMPLATE _Rng_from_urng
	template <class _Diff, class _Urng>
	class _Rng_from_urng { // wrap a URNG as an RNG
	public:
		using _Ty0 = std::make_unsigned_t<_Diff>;
		using _Ty1 = typename _Urng::result_type;

		using _Udiff = std::conditional_t < sizeof(_Ty1) < sizeof(_Ty0), _Ty0, _Ty1 > ;

		explicit _Rng_from_urng(_Urng& _Func) : _Ref(_Func), _Bits(8 * sizeof(_Udiff)), _Bmask(_Udiff(-1)) {
			for (; (_Urng::max)() - (_Urng::min)() < _Bmask; _Bmask >>= 1) {
				--_Bits;
			}
		}

		_Diff operator()(_Diff _Index) { // adapt _Urng closed range to [0, _Index)
			for (;;) { // try a sample random value
				_Udiff _Ret = 0; // random bits
				_Udiff _Mask = 0; // 2^N - 1, _Ret is within [0, _Mask]

				while (_Mask < _Udiff(_Index - 1)) { // need more random bits
					_Ret <<= _Bits - 1; // avoid full shift
					_Ret <<= 1;
					_Ret |= _Get_bits();
					_Mask <<= _Bits - 1; // avoid full shift
					_Mask <<= 1;
					_Mask |= _Bmask;
				}

				// _Ret is [0, _Mask], _Index - 1 <= _Mask, return if unbiased
				if (_Ret / _Index < _Mask / _Index || _Mask % _Index == _Udiff(_Index - 1)) {
					return static_cast<_Diff>(_Ret % _Index);
				}
			}
		}

		_Udiff _Get_all_bits() { // return a random value
			_Udiff _Ret = 0;

			for (size_t _Num = 0; _Num < 8 * sizeof(_Udiff); _Num += _Bits) { // don't mask away any bits
				_Ret <<= _Bits - 1; // avoid full shift
				_Ret <<= 1;
				_Ret |= _Get_bits();
			}

			return _Ret;
		}

		_Rng_from_urng(const _Rng_from_urng&) = delete;
		_Rng_from_urng& operator=(const _Rng_from_urng&) = delete;

	private:
		_Udiff _Get_bits() { // return a random value within [0, _Bmask]
			for (;;) { // repeat until random value is in range
				_Udiff _Val = _Ref() - (_Urng::min)();

				if (_Val <= _Bmask) {
					return _Val;
				}
			}
		}

		_Urng& _Ref; // reference to URNG
		size_t _Bits; // number of random bits generated by _Get_bits()
		_Udiff _Bmask; // 2^_Bits - 1
	};

	// CLASS TEMPLATE uniform_int
	template <class _Ty = int>
	class uniform_int { // uniform integer distribution
	public:
		using result_type = _Ty;

		struct param_type { // parameter package
			using distribution_type = uniform_int;

			explicit param_type(result_type _Min0 = 0, result_type _Max0 = 9) { // construct from parameters
				_Init(_Min0, _Max0);
			}

			bool operator==(const param_type& _Right) const { // test for equality
				return _Min == _Right._Min && _Max == _Right._Max;
			}

			bool operator!=(const param_type& _Right) const { // test for inequality
				return !(*this == _Right);
			}

			result_type a() const { // return a value
				return _Min;
			}

			result_type b() const { // return b value
				return _Max;
			}

			void _Init(_Ty _Min0, _Ty _Max0) { // set internal state
				_Min = _Min0;
				_Max = _Max0;
			}

			result_type _Min;
			result_type _Max;
		};

		explicit uniform_int(_Ty _Min0 = 0, _Ty _Max0 = 9) : _Par(_Min0, _Max0) { // construct from parameters
		}

		explicit uniform_int(const param_type& _Par0) : _Par(_Par0) { // construct from parameter package
		}

		result_type a() const { // return a value
			return _Par.a();
		}

		result_type b() const { // return b value
			return _Par.b();
		}

		param_type param() const { // return parameter package
			return _Par;
		}

		void param(const param_type& _Par0) { // set parameter package
			_Par = _Par0;
		}

		result_type(min)() const { // return minimum possible generated value
			return _Par._Min;
		}

		result_type(max)() const { // return maximum possible generated value
			return _Par._Max;
		}

		void reset() { // clear internal state
		}

		template <class _Engine>
		result_type operator()(_Engine& _Eng) const { // return next value
			return _Eval(_Eng, _Par._Min, _Par._Max);
		}

		template <class _Engine>
		result_type operator()(
			_Engine& _Eng, const param_type& _Par0) const { // return next value, given parameter package
			return _Eval(_Eng, _Par0._Min, _Par0._Max);
		}

		template <class _Engine>
		result_type operator()(_Engine& _Eng, result_type _Nx) const { // return next value
			return _Eval(_Eng, 0, _Nx - 1);
		}

		template <class _Elem, class _Traits>
		std::basic_istream<_Elem, _Traits>& _Read(std::basic_istream<_Elem, _Traits>& _Istr) { // read state from _Istr
			_Ty _Min0;
			_Ty _Max0;
			_Istr >> _Min0 >> _Max0;
			_Par._Init(_Min0, _Max0);
			return _Istr;
		}

		template <class _Elem, class _Traits>
		std::basic_ostream<_Elem, _Traits>& _Write(std::basic_ostream<_Elem, _Traits>& _Ostr) const { // write state to _Ostr
			return _Ostr << _Par._Min << ' ' << _Par._Max;
		}

	private:
		using _Uty = std::make_unsigned_t<_Ty>;

		template <class _Engine>
		result_type _Eval(_Engine& _Eng, _Ty _Min, _Ty _Max) const { // compute next value in range [_Min, _Max]
			_Rng_from_urng<_Uty, _Engine> _Rng(_Eng);

			const _Uty _Umin = _Adjust(_Uty(_Min));
			const _Uty _Umax = _Adjust(_Uty(_Max));

			_Uty _Uret;

			if (_Umax - _Umin == _Uty(-1)) {
				_Uret = static_cast<_Uty>(_Rng._Get_all_bits());
			}
			else {
				_Uret = static_cast<_Uty>(_Rng(static_cast<_Uty>(_Umax - _Umin + 1)));
			}

			return _Ty(_Adjust(static_cast<_Uty>(_Uret + _Umin)));
		}

		static _Uty _Adjust(_Uty _Uval) { // convert signed ranges to unsigned ranges and vice versa
			return _Adjust(_Uval, std::is_signed<_Ty>());
		}

		static _Uty _Adjust(_Uty _Uval, std::true_type) { // convert signed ranges to unsigned ranges and vice versa
			const _Uty _Adjuster = (_Uty(-1) >> 1) + 1; // 2^(N-1)

			if (_Uval < _Adjuster) {
				return static_cast<_Uty>(_Uval + _Adjuster);
			}
			else {
				return static_cast<_Uty>(_Uval - _Adjuster);
			}
		}

		static _Uty _Adjust(_Uty _Uval, std::false_type) { // _Ty is already unsigned, do nothing
			return _Uval;
		}

		param_type _Par;
	};

	template <class _Elem, class _Traits, class _Ty>
	std::basic_istream<_Elem, _Traits>& operator>>(std::basic_istream<_Elem, _Traits>& _Istr,
		uniform_int<_Ty>& _Dist) { // read state from _Istr
		return _Dist._Read(_Istr);
	}

	template <class _Elem, class _Traits, class _Ty>
	std::basic_ostream<_Elem, _Traits>& operator<<(std::basic_ostream<_Elem, _Traits>& _Ostr,
		const uniform_int<_Ty>& _Dist) { // write state to _Ostr
		return _Dist._Write(_Ostr);
	}

	// CLASS TEMPLATE uniform_int_distribution
	template <class _Ty = int>
	class uniform_int_distribution : public uniform_int<_Ty> { // uniform integer distribution
	public:
		using _Mybase     = uniform_int<_Ty>;
		using _Mypbase    = typename _Mybase::param_type;
		using result_type = typename _Mybase::result_type;

		struct param_type : public _Mypbase { // parameter package
			using distribution_type = uniform_int_distribution;

			explicit param_type(result_type _Min0 = 0, result_type _Max0 = (std::numeric_limits<_Ty>::max)())
				: _Mypbase(_Min0, _Max0) { // construct from parameters
			}

			param_type(const _Mypbase& _Right) : _Mypbase(_Right) { // construct from base
			}
		};

		explicit uniform_int_distribution(_Ty _Min0 = 0, _Ty _Max0 = (std::numeric_limits<_Ty>::max)())
			: _Mybase(_Min0, _Max0) { // construct from parameters
		}

		explicit uniform_int_distribution(const param_type& _Par0) : _Mybase(_Par0) { // construct from parameter package
		}
	};

	template <class _Ty>
	bool operator==(const uniform_int_distribution<_Ty>& _Left,
		const uniform_int_distribution<_Ty>& _Right) { // test for equality
		return _Left.param() == _Right.param();
	}

	template <class _Ty>
	bool operator!=(const uniform_int_distribution<_Ty>& _Left,
		const uniform_int_distribution<_Ty>& _Right) { // test for inequality
		return !(_Left == _Right);
	}
}