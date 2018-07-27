#include "Pch.h"

#include "Engine/Types/Matrix.h"

const BaseMatrix2<float> BaseMatrix2<float>::Identity = BaseMatrix2<float>(
	1.0f, 0.0f,
	0.0f, 1.0f
);

const BaseMatrix2<float> BaseMatrix2<float>::Zero = BaseMatrix2<float>(
	0.0f, 0.0f,
	0.0f, 0.0f
);

const BaseMatrix3<float> BaseMatrix3<float>::Identity = BaseMatrix3<float>(
	1.0f, 0.0f, 0.0f,
	0.0f, 1.0f, 0.0f,
	0.0f, 0.0f, 1.0f
);

const BaseMatrix3<float> BaseMatrix3<float>::Zero = BaseMatrix3<float>(
	0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f
);

const BaseMatrix4<float> BaseMatrix4<float>::Identity = BaseMatrix4<float>(
	1.0f, 0.0f, 0.0f, 0.0f,
	0.0f, 1.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 1.0f, 0.0f,
	0.0f, 0.0f, 0.0f, 1.0f
);

const BaseMatrix4<float> BaseMatrix4<float>::Zero = BaseMatrix4<float>(
	0.0f, 0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f, 0.0f
);
