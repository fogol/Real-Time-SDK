/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.access;

import java.util.ArrayList;
import java.util.List;

class NiProviderSessionChannelInfo<T> extends BaseSessionChannelInfo<T>
{
	boolean _sentDirectory;
	
	List<PackedMsgImpl> _allocatedPackedBuffers;
	
	NiProviderSessionChannelInfo(NiProviderSessionChannelConfig niProviderSessionConfig, NiProviderSession<T> niProviderSession)
	{
		super(niProviderSessionConfig, niProviderSession);
		_sentDirectory = false;
		_allocatedPackedBuffers = new ArrayList<PackedMsgImpl>();
	}
	
	public boolean sentDirectory()
	{
		return _sentDirectory;
	}
	
	public void sentDirectory(boolean sentDirectory)
	{
		_sentDirectory = sentDirectory;
	}
	
	public List<PackedMsgImpl> packedBufferList()
	{
		return _allocatedPackedBuffers;
	}
}