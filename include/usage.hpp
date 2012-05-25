//-----------------------------------------------------------------------------------------
#if 0

fix8 is released under the New BSD License.

Copyright (c) 2010-12, David L. Dight <fix@fix8.org>
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are
permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of
	 	conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list
	 	of conditions and the following disclaimer in the documentation and/or other
		materials provided with the distribution.
    * Neither the name of the author nor the names of its contributors may be used to
	 	endorse or promote products derived from this software without specific prior
		written permission.
    * Products derived from this software may not be called "Fix8", nor can "Fix8" appear
	   in their name without written permission from fix8.org

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS
OR  IMPLIED  WARRANTIES,  INCLUDING,  BUT  NOT  LIMITED  TO ,  THE  IMPLIED  WARRANTIES  OF
MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE ARE  DISCLAIMED. IN  NO EVENT  SHALL
THE  COPYRIGHT  OWNER OR  CONTRIBUTORS BE  LIABLE  FOR  ANY DIRECT,  INDIRECT,  INCIDENTAL,
SPECIAL,  EXEMPLARY, OR CONSEQUENTIAL  DAMAGES (INCLUDING,  BUT NOT LIMITED TO, PROCUREMENT
OF SUBSTITUTE  GOODS OR SERVICES; LOSS OF USE, DATA,  OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED  AND ON ANY THEORY OF LIABILITY, WHETHER  IN CONTRACT, STRICT  LIABILITY, OR
TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE
EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#endif
//-----------------------------------------------------------------------------------------
#ifndef _USAGE_HPP_
#define _USAGE_HPP_

//-----------------------------------------------------------------------------------------
/// Convenient program help/usage wrapper. Generates a standardised usage message.
class UsageMan
{
	const std::string prognm_, params_;
	std::string argstr_, description_;
	typedef std::map<const char, const std::pair<const std::string, const std::string> > OPTEL;
	OPTEL optels_;
	std::list<std::string> xtrlines_;
	const int splen_, argoptlen_;

public:
	/*! Ctor.
	  \param prognm program name
	  \param argstr program arguments string
	  \param params program non-switched arguments string
	  \param splen tab width
	  \param argoptlen option argument width */
	UsageMan(const std::string& prognm, const std::string& argstr, const std::string& params,
		int splen=3, int argoptlen=23)
		: prognm_(prognm), params_(params), argstr_(argstr), splen_(splen), argoptlen_(argoptlen)
	{
		std::sort(argstr_.begin(), argstr_.end());
	}

	/// Dtor.
	virtual ~UsageMan() {}

	/*! Add a command line option.
	  \param sw the single character switch
	  \param lsw the string switch (long version)
	  \param help the associated help string
	  \return true on success */
	bool add(const char sw, const std::string& lsw, const std::string& help)
	{
		return optels_.insert(OPTEL::value_type(sw,
			std::pair<const std::string, const std::string>(lsw, help))).second;
	}

	/*! Add an extra help line. Lines prefixed with '@' are indented one tabstop.
	  \param xtr the extra help line */
	void add(const std::string& xtr)
	{
		std::string topush(xtr);
		std::string::size_type spos;
		while ((spos = topush.find_first_of('@')) != std::string::npos)
			topush.replace(spos, 1, std::string(splen_, ' '));
		xtrlines_.push_back(topush);
	}

	/*! Set the usage description string.
	  \param desc the description string */
	void setdesc(const std::string& desc) { description_ = desc; }

	/*! Insert the formatted usage into the given output stream.
	  \param os the stream to insert to */
	void print(std::ostream& os) const
	{
		if (!description_.empty())
			os << description_ << std::endl << std::endl;
		os << "Usage: " << prognm_ << " [-";
		for (std::string::const_iterator itr(argstr_.begin()); itr != argstr_.end(); ++itr)
			if (*itr != ':')
				os << *itr;
		os << "] " << params_ << std::endl;
		const std::string spacer(splen_, ' ');
		for (OPTEL::const_iterator itr(optels_.begin()); itr != optels_.end(); ++itr)
		{
			std::ostringstream ostr;
			ostr << '-' << itr->first << ",--" << itr->second.first;
			os << spacer << std::left << std::setw(argoptlen_) << ostr.str()
				<< ' ' << itr->second.second << std::endl;
		}
		for (std::list<std::string>::const_iterator itr(xtrlines_.begin()); itr != xtrlines_.end(); ++itr)
			os << *itr << std::endl;
	}
};

#endif // _USAGE_HPP_

